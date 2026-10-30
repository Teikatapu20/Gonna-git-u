#ifndef INVENTORY_H
#define INVENTORY_H

#include "Product.h"
#include <string>
#include <vector>

class Inventory {
public:
    bool addProduct(const Product& product, std::string& message);
    Product* findById(const std::string& id);
    const Product* findById(const std::string& id) const;
    std::vector<Product> search(const std::string& query) const;
    bool updateProduct(const std::string& id, const Product& replacement);
    bool deleteProduct(const std::string& id);
    const std::vector<Product>& getProducts() const;
    void setProducts(const std::vector<Product>& products);
    double inventoryValue() const;
    long long totalUnits() const;
    std::vector<Product> lowStockProducts() const;

private:
    std::vector<Product> products;
};

#endif
