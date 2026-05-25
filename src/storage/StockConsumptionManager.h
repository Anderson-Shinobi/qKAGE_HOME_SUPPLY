#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../core/InventoryItem.h"

struct StockConsumptionResult {
    bool success = false;
    std::string message;
    std::string item;
    double previousQuantity = 0.0;
    double consumedQuantity = 0.0;
    double currentQuantity = 0.0;
    std::string unit;
};

class StockConsumptionManager {
public:
    StockConsumptionResult consume(
        const std::filesystem::path &inventoryCsvPath,
        const std::filesystem::path &historyCsvPath,
        const std::string &itemName,
        double quantity,
        const std::string &observation) const;

private:
    static int writeInventoryCsv(const std::filesystem::path &path, const std::vector<InventoryItem> &items);
    static std::string escapeField(const std::string &value);
    static bool isBlank(const std::string &value);
};
