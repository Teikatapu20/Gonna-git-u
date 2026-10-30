#ifndef STORAGE_H
#define STORAGE_H

#include "Product.h"
#include <string>
#include <vector>

class Storage {
public:
    static bool load(const std::string& filename, std::vector<Product>& products, std::string& message);
    static bool save(const std::string& filename, const std::vector<Product>& products, std::string& message);
};

#endif
