#pragma once

#include <filesystem>
#include <string>

struct MonthlyReportResult {
    bool success = false;
    std::string message;
    std::string report;
};

class MonthlyReportGenerator {
public:
    MonthlyReportResult generate(
        const std::filesystem::path &inventoryCsvPath,
        const std::filesystem::path &piggyBankCsvPath) const;
};
