#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>

struct Product {
    std::string id;
    std::string name;
    std::string category;
    std::string platform;
    double price = 0.0;
    int quantity = 0;
    int reorderLevel = 0;
};

#endif
