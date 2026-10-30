#include "Input.h"
#include "Inventory.h"
#include "Storage.h"
#include <cmath>
#include <iomanip>
#include <iostream>

namespace {
const std::string DATA_FILE = "products.csv";

void printProduct(const Product& p) {
    std::cout << std::left << std::setw(12) << p.id << std::setw(24) << p.name
              << std::setw(18) << p.category << std::setw(14) << p.platform
              << std::right << std::setw(10) << std::fixed << std::setprecision(2) << p.price
              << std::setw(9) << p.quantity << std::setw(12) << p.reorderLevel << '\n';
}

void printProducts(const std::vector<Product>& products) {
    if (products.empty()) { std::cout << "No matching products found.\n"; return; }
    std::cout << std::left << std::setw(12) << "ID" << std::setw(24) << "Name" << std::setw(18) << "Category"
              << std::setw(14) << "Platform" << std::right << std::setw(10) << "Price"
              << std::setw(9) << "Qty" << std::setw(12) << "Reorder" << "\n";
    std::cout << std::string(99, '-') << '\n';
    for (const auto& p : products) printProduct(p);
}

void addProduct(Inventory& inventory) {
    Product p;
    p.id = Input::text("Product ID: ");
    p.name = Input::text("Product name: ");
    p.category = Input::text("Category (game, console, accessory, etc.): ");
    p.platform = Input::text("Platform (PC, PlayStation, Xbox, Nintendo, etc.): ");
    p.price = Input::money("Unit price: ");
    p.quantity = Input::integer("Quantity in stock: ");
    p.reorderLevel = Input::integer("Low-stock threshold: ");
    std::string message;
    if (inventory.addProduct(p, message) && !Storage::save(DATA_FILE, inventory.getProducts(), message))
        std::cout << "Warning: " << message << " The change is currently in memory only.\n";
    else std::cout << message << '\n';
}

void updateProduct(Inventory& inventory) {
    const std::string id = Input::text("Enter product ID to update: ");
    const Product* found = inventory.findById(id);
    if (!found) { std::cout << "Product not found.\n"; return; }
    Product replacement = *found;
    std::cout << "Press Enter to keep a current value.\n";
    auto optionalText = [](const std::string& prompt, const std::string& current) {
        std::string value; std::cout << prompt << " [" << current << "]: "; std::getline(std::cin, value);
        return value.empty() ? current : value;
    };
    replacement.name = optionalText("Name", replacement.name);
    replacement.category = optionalText("Category", replacement.category);
    replacement.platform = optionalText("Platform", replacement.platform);
    std::string value;
    std::cout << "Unit price [" << replacement.price << "]: "; std::getline(std::cin, value);
    if (!value.empty()) { try { size_t n; double x = std::stod(value, &n); if (n != value.size() || !std::isfinite(x) || x < 0) throw 1; replacement.price = x; } catch (...) { std::cout << "Invalid price; keeping previous value.\n"; } }
    std::cout << "Quantity [" << replacement.quantity << "]: "; std::getline(std::cin, value);
    if (!value.empty()) { try { size_t n; int x = std::stoi(value, &n); if (n != value.size() || x < 0) throw 1; replacement.quantity = x; } catch (...) { std::cout << "Invalid quantity; keeping previous value.\n"; } }
    std::cout << "Low-stock threshold [" << replacement.reorderLevel << "]: "; std::getline(std::cin, value);
    if (!value.empty()) { try { size_t n; int x = std::stoi(value, &n); if (n != value.size() || x < 0) throw 1; replacement.reorderLevel = x; } catch (...) { std::cout << "Invalid threshold; keeping previous value.\n"; } }
    inventory.updateProduct(id, replacement);
    std::string message;
    if (!Storage::save(DATA_FILE, inventory.getProducts(), message)) std::cout << "Warning: " << message << '\n';
    else std::cout << "Product updated and saved.\n";
}

void showSummary(const Inventory& inventory) {
    std::cout << "\n--- Inventory Summary ---\n"
              << "Product records: " << inventory.getProducts().size() << '\n'
              << "Total units in stock: " << inventory.totalUnits() << '\n'
              << "Estimated stock value: " << std::fixed << std::setprecision(2) << inventory.inventoryValue() << '\n';
    const auto low = inventory.lowStockProducts();
    std::cout << "Products at/below reorder threshold: " << low.size() << '\n';
    if (!low.empty()) printProducts(low);
}

void printMenu() {
    std::cout << "\n=== GAMEHUB INVENTORY ===\n"
              << "1. Add product\n2. View all products\n3. Search products\n"
              << "4. Update product\n5. Delete product\n6. Generate summary\n0. Save and exit\n";
}
}

int main() {
    Inventory inventory;
    std::vector<Product> loaded;
    std::string message;
    if (!Storage::load(DATA_FILE, loaded, message)) {
        std::cerr << message << "\nProgram will stop to avoid overwriting the file.\n";
        return 1;
    }
    inventory.setProducts(loaded);
    std::cout << message << '\n';
    bool running = true;
    while (running) {
        printMenu();
        const int choice = Input::integer("Choose an option: ");
        switch (choice) {
            case 1: addProduct(inventory); break;
            case 2: printProducts(inventory.getProducts()); break;
            case 3: printProducts(inventory.search(Input::text("Search by ID, name, category, or platform: "))); break;
            case 4: updateProduct(inventory); break;
            case 5: {
                const std::string id = Input::text("Enter product ID to delete: ");
                const Product* p = inventory.findById(id);
                if (!p) std::cout << "Product not found.\n";
                else if (Input::yesNo("Delete " + p->name + " (" + p->id + ")?")) {
                    inventory.deleteProduct(id);
                    if (!Storage::save(DATA_FILE, inventory.getProducts(), message)) std::cout << "Warning: " << message << '\n';
                    else std::cout << "Product deleted.\n";
                } else std::cout << "Deletion cancelled.\n";
                break;
            }
            case 6: showSummary(inventory); break;
            case 0:
                if (Storage::save(DATA_FILE, inventory.getProducts(), message)) std::cout << message << '\n';
                else std::cout << "Warning: " << message << '\n';
                running = false; break;
            default: std::cout << "Choose a menu number from 0 to 6.\n";
        }
    }
    std::cout << "Goodbye.\n";
    return 0;
}
