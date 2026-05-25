#pragma once

#include <filesystem>
#include <string>

struct SystemConfig {
    std::string version;
    std::string currency;
    bool logsEnabled = true;
    bool backupEnabled = true;
    bool reportsEnabled = true;
    bool debugMode = false;
};

struct PathConfig {
    std::string dataDir;
    std::string backupDir;
    std::string reportsDir;
};

struct LimitConfig {
    double criticalAutonomyMonths = 0.0;
    double warningAutonomyMonths = 0.0;
    int expirationCriticalDays = 0;
    int expirationWarningDays = 0;
};

struct AppConfig {
    SystemConfig system;
    PathConfig paths;
    LimitConfig limits;
};

struct ConfigResult {
    bool success = false;
    std::string message;
    AppConfig config;
};

class ConfigManager {
public:
    ConfigResult loadOrCreate(const std::filesystem::path &configPath) const;

private:
    static bool writeDefaultConfig(const std::filesystem::path &configPath, std::string &message);
    static ConfigResult load(const std::filesystem::path &configPath);
};
