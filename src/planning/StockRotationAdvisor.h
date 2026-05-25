#pragma once

#include <filesystem>
#include <string>
#include <ctime>
#include <vector>

struct StockRotationAdvice {
    std::string item;
    std::string category;
    double quantity = 0.0;
    std::string expirationDate;
    double autonomyMonths = 0.0;
    std::string priority;
    std::string recommendation;
};

struct StockRotationResult {
    bool success = false;
    std::string message;
    std::vector<StockRotationAdvice> items;
};

class StockRotationAdvisor {
public:
    StockRotationResult advise(const std::filesystem::path &inventoryCsvPath) const;

private:
    static bool parseDate(const std::string &value, std::tm &date);
    static int daysUntil(const std::tm &date);
};
