#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../core/InventoryItem.h"
#include "CsvInventoryReader.h"

class CsvInventoryWriter {
public:
    CsvResult addItem(const std::filesystem::path &path, const InventoryItem &item) const;
    CsvResult updateItem(const std::filesystem::path &path, const std::string &originalName, const InventoryItem &item) const;
    CsvResult removeItem(const std::filesystem::path &path, const std::string &itemName) const;

private:
    static CsvResult validateItem(const InventoryItem &item);
    static CsvResult writeItems(const std::filesystem::path &path, const std::vector<InventoryItem> &items);
    static std::string escapeField(const std::string &value);
};
