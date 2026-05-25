#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct ExpirationItem {
    std::string item;
    std::string category;
    double quantity = 0.0;
    std::string expirationDate;
    int remainingDays = 0;
    std::string status;
};

struct ExpirationAnalysisResult {
    bool success = false;
    std::string message;
    std::vector<ExpirationItem> items;
};

class ExpirationAnalyzer {
public:
    ExpirationAnalysisResult analyze(const std::filesystem::path &inventoryCsvPath) const;

private:
    static bool parseDate(const std::string &value, std::tm &date);
    static int daysUntil(const std::tm &date);
    static std::string classify(int remainingDays, const std::string &expirationDate);
};
