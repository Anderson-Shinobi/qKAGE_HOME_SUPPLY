#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct DataIntegrityOptions {
    bool fix = false;
    std::filesystem::path rootPath = ".";
};

struct DataIntegrityReport {
    bool success = false;
    std::string status;
    std::vector<std::string> checkedFiles;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::vector<std::string> fixes;
};

class DataIntegrityChecker {
public:
    DataIntegrityReport check(const DataIntegrityOptions &options) const;

private:
    static void validateInventoryCsv(const std::filesystem::path &path, DataIntegrityReport &report);
    static void validatePiggyBankCsv(const std::filesystem::path &path, DataIntegrityReport &report);
    static void validateCsv(
        const std::filesystem::path &path,
        const std::string &expectedHeader,
        int expectedColumns,
        const std::vector<int> &nonNegativeNumericColumns,
        const std::vector<int> &positiveNumericColumns,
        DataIntegrityReport &report);
    static void applyFixes(
        const std::filesystem::path &rootPath,
        const std::filesystem::path &inventoryPath,
        const std::filesystem::path &piggyBankPath,
        DataIntegrityReport &report);
    static void ensureCsvHeader(const std::filesystem::path &path, const std::string &header, DataIntegrityReport &report);
    static void removeBlankLines(const std::filesystem::path &path, DataIntegrityReport &report);
    static std::vector<std::string> parseLine(const std::string &line);
    static bool isBlank(const std::string &value);
    static bool isNumber(const std::string &value);
};
