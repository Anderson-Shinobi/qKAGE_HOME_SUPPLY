#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../core/InventoryItem.h"

struct CsvResult {
    bool success = false;
    std::string message;
};

struct CsvReadResult {
    bool success = false;
    std::string message;
    std::vector<InventoryItem> items;
};

class CsvInventoryReader {
public:
    CsvResult ensureFile(const std::filesystem::path &path) const;
    CsvReadResult read(const std::filesystem::path &path) const;

private:
    static std::vector<std::string> parseLine(const std::string &line);
    static double parseNumber(const std::string &value);
};
