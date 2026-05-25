#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct StockMovementResult {
    bool success = false;
    std::string message;
};

struct StockMovement {
    std::string dateTime;
    std::string type;
    std::string item;
    double quantity = 0.0;
    std::string unit;
    std::string observation;
};

struct StockMovementReadResult {
    bool success = false;
    std::string message;
    std::vector<StockMovement> movements;
};

class StockMovementHistory {
public:
    StockMovementResult ensureFile(const std::filesystem::path &path) const;
    StockMovementResult record(
        const std::filesystem::path &path,
        const std::string &type,
        const std::string &item,
        double quantity,
        const std::string &unit,
        const std::string &observation) const;
    StockMovementReadResult readAll(const std::filesystem::path &path) const;

private:
    static StockMovementResult validate(const std::string &type, const std::string &item, double quantity);
    static bool isAllowedType(const std::string &type);
    static bool isBlank(const std::string &value);
    static std::string currentDateTime();
    static std::string escapeField(const std::string &value);
    static std::vector<std::string> parseLine(const std::string &line);
    static double parseNumber(const std::string &value);
};
