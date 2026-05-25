#pragma once

#include <string>
#include <vector>

#include "../core/InventoryItem.h"

struct StockAutonomyEntry {
    std::string item;
    double quantity = 0.0;
    double monthlyConsumption = 0.0;
    double autonomyMonths = 0.0;
    std::string status;
};

struct StockAutonomyAnalysis {
    bool success = false;
    std::string message;
    std::vector<StockAutonomyEntry> entries;
};

StockAutonomyAnalysis analyzeStockAutonomy(const std::vector<InventoryItem> &items);
