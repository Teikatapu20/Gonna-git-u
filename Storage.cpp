#include "Storage.h"

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>

namespace {

const char* const HEADER = "id,name,category,platform,price,quantity,reorder_level";
const std::size_t FIELD_COUNT = 7;
const double MAX_PRICE = 1000000000.0;

std::string trim(const std::string& text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

std::string toLower(std::string text) {
    for (char& c : text) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

bool parseInt(const std::string& text, int& result) {
    std::size_t i = 0;
    bool negative = false;
    if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
        negative = (text[i] == '-');
        ++i;
    }
    if (i == text.size()) return false;

    long long value = 0;
    for (; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
        value = value * 10 + (text[i] - '0');
        if (value > std::numeric_limits<int>::max()) return false;
    }
    result = static_cast<int>(negative ? -value : value);
    return true;
}

bool parseMoney(const std::string& text, double& result) {
    bool seenDigit = false;
    bool seenDot = false;
    for (char c : text) {
        if (std::isdigit(static_cast<unsigned char>(c))) seenDigit = true;
        else if (c == '.' && !seenDot) seenDot = true;
        else return false;
    }
    if (!seenDigit) return false;

    const double value = std::strtod(text.c_str(), nullptr);
    if (!std::isfinite(value) || value > MAX_PRICE) return false;
    result = value;
    return true;
}

// Wraps a field in quotes and doubles any quote inside it, so names with
// commas or quotes survive a save and load unchanged.
std::string quote(const std::string& text) {
    std::string out = "\"";
    for (char c : text) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += '"';
    return out;
}

struct CsvRow {
    std::vector<std::string> fields;
    std::size_t line;  // line number where the row starts, for error messages
};

// Reads CSV text into rows. Handles quoted fields, "" for a literal quote,
// commas inside quotes, CRLF line endings, and blank lines.
bool parseCsv(const std::string& content, std::vector<CsvRow>& rows, std::string& error) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    bool fieldWasQuoted = false;
    bool justClosedQuote = false;
    std::size_t line = 1;
    std::size_t rowStartLine = 1;

    auto endField = [&]() {
        fields.push_back(field);
        field.clear();
        fieldWasQuoted = false;
        justClosedQuote = false;
    };
    auto endRow = [&]() {
        const bool blank = fields.empty() && field.empty() && !fieldWasQuoted;
        if (!blank) {
            endField();
            CsvRow row;
            row.fields = fields;
            row.line = rowStartLine;
            rows.push_back(row);
        }
        fields.clear();
        field.clear();
        fieldWasQuoted = false;
        justClosedQuote = false;
    };

    for (std::size_t i = 0; i < content.size(); ++i) {
        const char c = content[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < content.size() && content[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = false;
                    justClosedQuote = true;
                }
            } else {
                if (c == '\n') ++line;
                field += c;
            }
            continue;
        }

        if (justClosedQuote && c != ',' && c != '\n' && c != '\r') {
            error = "line " + std::to_string(line) + " has text right after a closing quote";
            return false;
        }
        if (c == '"') {
            if (!field.empty() || fieldWasQuoted) {
                error = "line " + std::to_string(line) + " has a stray quote inside a field";
                return false;
            }
            inQuotes = true;
            fieldWasQuoted = true;
        } else if (c == ',') {
            endField();
        } else if (c == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
            continue;  // the '\n' that follows ends the row
        } else if (c == '\n' || c == '\r') {
            endRow();
            ++line;
            rowStartLine = line;
        } else {
            field += c;
        }
    }
    if (inQuotes) {
        error = "a quoted field starting near line " + std::to_string(rowStartLine) + " is never closed";
        return false;
    }
    endRow();  // last row when the file does not end with a newline
    return true;
}

}  // namespace

bool Storage::load(const std::string& filename, std::vector<Product>& products, std::string& message) {
    std::ifstream in(filename.c_str(), std::ios::binary);
    if (!in) {
        errno = 0;
        std::FILE* probe = std::fopen(filename.c_str(), "rb");
        if (probe == nullptr && errno == ENOENT) {
            products.clear();
            message = "No saved data found (" + filename + "). Starting with an empty inventory.";
            return true;  // first run: the file is created on the first save
        }
        if (probe != nullptr) std::fclose(probe);
        message = "Could not open " + filename + " for reading. Close it in other programs and check its permissions.";
        return false;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    std::string content = buffer.str();
    if (content.compare(0, 3, "\xEF\xBB\xBF") == 0) content.erase(0, 3);  // BOM added by some editors

    std::vector<CsvRow> rows;
    std::string error;
    if (!parseCsv(content, rows, error)) {
        message = "Could not read " + filename + ": " + error + ".";
        return false;
    }

    std::size_t first = 0;
    if (!rows.empty() && rows[0].fields.size() >= 2 &&
        toLower(trim(rows[0].fields[0])) == "id" && toLower(trim(rows[0].fields[1])) == "name") {
        first = 1;  // skip the header row
    }

    std::vector<Product> loaded;
    std::set<std::string> seenIds;
    for (std::size_t r = first; r < rows.size(); ++r) {
        const CsvRow& row = rows[r];
        const std::string where = filename + " line " + std::to_string(row.line) + ": ";
        const std::string fix = " Fix or remove that row, then start the program again.";

        if (row.fields.size() != FIELD_COUNT) {
            message = where + "expected " + std::to_string(FIELD_COUNT) + " fields but found " +
                      std::to_string(row.fields.size()) + "." + fix;
            return false;
        }

        Product p;
        p.id = trim(row.fields[0]);
        p.name = trim(row.fields[1]);
        p.category = trim(row.fields[2]);
        p.platform = trim(row.fields[3]);
        if (p.id.empty() || p.name.empty()) {
            message = where + "ID and name cannot be blank." + fix;
            return false;
        }
        if (!parseMoney(trim(row.fields[4]), p.price)) {
            message = where + "price '" + row.fields[4] + "' is not a valid price." + fix;
            return false;
        }
        if (!parseInt(trim(row.fields[5]), p.quantity) || p.quantity < 0) {
            message = where + "quantity '" + row.fields[5] + "' must be a whole number, zero or higher." + fix;
            return false;
        }
        if (!parseInt(trim(row.fields[6]), p.reorderLevel) || p.reorderLevel < 0) {
            message = where + "reorder level '" + row.fields[6] + "' must be a whole number, zero or higher." + fix;
            return false;
        }
        if (!seenIds.insert(toLower(p.id)).second) {
            message = where + "duplicate product ID '" + p.id + "'." + fix;
            return false;
        }
        loaded.push_back(p);
    }

    products = loaded;
    message = "Loaded " + std::to_string(products.size()) + " product(s) from " + filename + ".";
    return true;
}

bool Storage::save(const std::string& filename, const std::vector<Product>& products, std::string& message) {
    // Write to a temporary file first so a failed save never damages the real data file.
    const std::string temp = filename + ".tmp";
    {
        std::ofstream out(temp.c_str(), std::ios::binary | std::ios::trunc);
        if (!out) {
            message = "Could not write " + temp + ". Check that the folder is writable.";
            return false;
        }
        out << HEADER << '\n';
        for (const Product& p : products) {
            out << quote(p.id) << ',' << quote(p.name) << ',' << quote(p.category) << ','
                << quote(p.platform) << ',' << std::fixed << std::setprecision(2) << p.price << ','
                << p.quantity << ',' << p.reorderLevel << '\n';
        }
        out.flush();
        if (!out) {
            out.close();
            std::remove(temp.c_str());
            message = "Writing to " + temp + " failed. The disk may be full.";
            return false;
        }
    }

    if (std::rename(temp.c_str(), filename.c_str()) != 0) {
        std::remove(filename.c_str());  // Windows cannot rename over an existing file
        if (std::rename(temp.c_str(), filename.c_str()) != 0) {
            message = "Could not replace " + filename + ". Your latest data is in " + temp + ".";
            return false;
        }
    }
    message = "Saved " + std::to_string(products.size()) + " product(s) to " + filename + ".";
    return true;
}
