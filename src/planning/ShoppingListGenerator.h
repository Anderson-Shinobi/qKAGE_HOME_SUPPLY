#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct ShoppingListItem {
    std::string item;
    std::string category;
    double currentQuantity = 0.0;
    double minimumStock = 0.0;
    double monthlyConsumption = 0.0;
    double estimatedAutonomy = 0.0;
    double suggestedPurchase = 0.0;
    double unitPrice = 0.0;
    double estimatedCost = 0.0;
    std::string unit;
};

struct ShoppingListResult {
    bool success = false;
    std::string message;
    std::vector<ShoppingListItem> items;
};

class ShoppingListGenerator {
public:
    ShoppingListResult generate(const std::filesystem::path &inventoryCsvPath) const;
};
