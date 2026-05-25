#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct PiggyBankResult {
    bool success = false;
    std::string message;
};

struct PiggyBank {
    std::string name;
    double currentValue = 0.0;
    double goal = 0.0;
    double monthlyContribution = 0.0;
    double goalPercentage = 0.0;
    std::string status;
};

struct PiggyBankReadResult {
    bool success = false;
    std::string message;
    std::vector<PiggyBank> piggyBanks;
};

class PiggyBankManager {
public:
    PiggyBankResult ensureFile(const std::filesystem::path &path) const;
    PiggyBankResult addPiggyBank(const std::filesystem::path &path, const PiggyBank &piggyBank) const;
    PiggyBankReadResult readAll(const std::filesystem::path &path) const;

private:
    static PiggyBankResult validatePiggyBank(const PiggyBank &piggyBank);
    static std::string classifyStatus(double goalPercentage);
    static std::string escapeField(const std::string &value);
    static std::vector<std::string> parseLine(const std::string &line);
    static double parseNumber(const std::string &value);
};
