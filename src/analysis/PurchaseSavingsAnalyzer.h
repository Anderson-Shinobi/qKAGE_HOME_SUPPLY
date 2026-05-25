#pragma once

#include <string>

struct PurchaseSavingsInput {
    std::string item;
    double quantity = 0.0;
    double localUnitPrice = 0.0;
    double wholesaleUnitPrice = 0.0;
};

struct PurchaseSavingsAnalysis {
    bool success = false;
    std::string message;
    std::string item;
    double quantity = 0.0;
    double localUnitPrice = 0.0;
    double wholesaleUnitPrice = 0.0;
    double unitSavings = 0.0;
    double totalSavings = 0.0;
    std::string status;
};

PurchaseSavingsAnalysis analyzePurchaseSavings(const PurchaseSavingsInput &input);
