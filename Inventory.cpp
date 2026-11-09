#include "Inventory.h"

#include <cctype>
#include <cmath>

namespace {

// Remove leading and trailing whitespace.
std::string trim(const std::string& text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

// Lowercase copy, used for case-insensitive comparisons.
std::string toLower(std::string text) {
    for (char& c : text) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

// IDs match when they are equal after trimming and ignoring case,
// so "A1" and " a1 " count as the same product.
bool sameId(const std::string& a, const std::string& b) {
    return toLower(trim(a)) == toLower(trim(b));
}

bool containsIgnoreCase(const std::string& text, const std::string& lowerQuery) {
    return toLower(text).find(lowerQuery) != std::string::npos;
}

// Shared rules for add and update. Returns true when the product is valid;
// otherwise puts a readable reason in `message`.
bool validate(const Product& p, std::string& message) {
    if (trim(p.id).empty()) { message = "Product ID cannot be empty."; return false; }
    if (trim(p.name).empty()) { message = "Product name cannot be empty."; return false; }
    if (!std::isfinite(p.price) || p.price < 0.0) { message = "Price must be zero or higher."; return false; }
    if (p.quantity < 0) { message = "Quantity cannot be negative."; return false; }
    if (p.reorderLevel < 0) { message = "Low-stock threshold cannot be negative."; return false; }
    return true;
}

// Copy of the product with surrounding whitespace removed from text fields.
Product cleaned(const Product& p) {
    Product c = p;
    c.id = trim(p.id);
    c.name = trim(p.name);
    c.category = trim(p.category);
    c.platform = trim(p.platform);
    return c;
}

}  // namespace

bool Inventory::addProduct(const Product& product, std::string& message) {
    if (!validate(product, message)) return false;

    Product item = cleaned(product);
    if (findById(item.id) != nullptr) {
        message = "A product with ID '" + item.id + "' already exists. Duplicate IDs are not allowed.";
        return false;
    }

    products.push_back(item);
    message = "Product added successfully.";
    return true;
}

Product* Inventory::findById(const std::string& id) {
    for (Product& p : products) {
        if (sameId(p.id, id)) return &p;
    }
    return nullptr;
}

const Product* Inventory::findById(const std::string& id) const {
    for (const Product& p : products) {
        if (sameId(p.id, id)) return &p;
    }
    return nullptr;
}

std::vector<Product> Inventory::search(const std::string& query) const {
    const std::string needle = toLower(trim(query));
    if (needle.empty()) return products;  // blank search lists everything

    std::vector<Product> results;
    for (const Product& p : products) {
        if (containsIgnoreCase(p.id, needle) || containsIgnoreCase(p.name, needle) ||
            containsIgnoreCase(p.category, needle) || containsIgnoreCase(p.platform, needle)) {
            results.push_back(p);
        }
    }
    return results;
}

bool Inventory::updateProduct(const std::string& id, const Product& replacement) {
    Product* existing = findById(id);
    if (existing == nullptr) return false;

    // The ID is the record's identity, so it never changes on update.
    Product updated = replacement;
    updated.id = existing->id;

    std::string ignored;
    if (!validate(updated, ignored)) return false;

    *existing = cleaned(updated);
    return true;
}

bool Inventory::deleteProduct(const std::string& id) {
    for (std::size_t i = 0; i < products.size(); ++i) {
        if (sameId(products[i].id, id)) {
            products.erase(products.begin() + static_cast<std::ptrdiff_t>(i));
            return true;
        }
    }
    return false;
}

const std::vector<Product>& Inventory::getProducts() const {
    return products;
}

void Inventory::setProducts(const std::vector<Product>& newProducts) {
    products = newProducts;
}

double Inventory::inventoryValue() const {
    double total = 0.0;
    for (const Product& p : products) total += p.price * p.quantity;
    return total;
}

long long Inventory::totalUnits() const {
    long long total = 0;
    for (const Product& p : products) total += p.quantity;
    return total;
}

std::vector<Product> Inventory::lowStockProducts() const {
    std::vector<Product> low;
    for (const Product& p : products) {
        if (p.quantity <= p.reorderLevel) low.push_back(p);  // "at or below" the threshold
    }
    return low;
}