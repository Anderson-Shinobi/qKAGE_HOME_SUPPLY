#pragma once

#include <string>

struct InventoryItem {
    std::string name;
    std::string category;
    double quantity = 0.0;
    std::string unit;
    double monthlyConsumption = 0.0;
    double unitPrice = 0.0;
    std::string expirationDate;
    double minimumStock = 0.0;

    double autonomyInMonths() const;
    double monthlyCost() const;
    bool isCritical() const;
};
