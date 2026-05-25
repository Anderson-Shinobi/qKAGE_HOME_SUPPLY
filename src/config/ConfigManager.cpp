#include "ConfigManager.h"

#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace {
const char *kDefaultConfigContent =
    "[system]\n"
    "version=0.1.0\n"
    "currency=BRL\n"
    "logs_enabled=true\n"
    "backup_enabled=true\n"
    "reports_enabled=true\n"
    "debug_mode=false\n"
    "\n"
    "[paths]\n"
    "data_dir=data\n"
    "backup_dir=backups\n"
    "reports_dir=reports\n"
    "\n"
    "[limits]\n"
    "critical_autonomy_months=1\n"
    "warning_autonomy_months=2\n"
    "expiration_critical_days=30\n"
    "expiration_warning_days=90\n";

std::string trim(const std::string &value)
{
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool parseBool(const std::string &value, bool &output)
{
    if (value == "true") {
        output = true;
        return true;
    }

    if (value == "false") {
        output = false;
        return true;
    }

    return false;
}

bool parseDouble(const std::string &value, double &output)
{
    try {
        std::size_t parsedCharacters = 0;
        output = std::stod(value, &parsedCharacters);
        return parsedCharacters == value.size();
    } catch (...) {
        return false;
    }
}

bool parseInt(const std::string &value, int &output)
{
    try {
        std::size_t parsedCharacters = 0;
        output = std::stoi(value, &parsedCharacters);
        return parsedCharacters == value.size();
    } catch (...) {
        return false;
    }
}

bool requireKey(
    const std::map<std::string, std::map<std::string, std::string>> &values,
    const std::string &section,
    const std::string &key,
    std::string &output,
    std::string &message)
{
    const auto sectionIt = values.find(section);
    if (sectionIt == values.end()) {
        message = "secao ausente: " + section;
        return false;
    }

    const auto keyIt = sectionIt->second.find(key);
    if (keyIt == sectionIt->second.end()) {
        message = "chave ausente: " + section + "." + key;
        return false;
    }

    output = keyIt->second;
    return true;
}
}

ConfigResult ConfigManager::loadOrCreate(const std::filesystem::path &configPath) const
{
    if (!std::filesystem::exists(configPath)) {
        std::string message;
        if (!writeDefaultConfig(configPath, message)) {
            return {false, message};
        }
    }

    return load(configPath);
}

bool ConfigManager::writeDefaultConfig(const std::filesystem::path &configPath, std::string &message)
{
    try {
        if (configPath.has_parent_path()) {
            std::filesystem::create_directories(configPath.parent_path());
        }

        std::ofstream file(configPath);
        if (!file.is_open()) {
            message = "nao foi possivel criar config: " + configPath.string();
            return false;
        }

        file << kDefaultConfigContent;
        message = "config criada com sucesso.";
        return true;
    } catch (const std::exception &error) {
        message = error.what();
        return false;
    }
}

ConfigResult ConfigManager::load(const std::filesystem::path &configPath)
{
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return {false, "arquivo inexistente ou sem permissao de leitura: " + configPath.string()};
    }

    const std::set<std::string> allowedSections = {"system", "paths", "limits"};
    const std::map<std::string, std::set<std::string>> allowedKeys = {
        {"system", {"version", "currency", "logs_enabled", "backup_enabled", "reports_enabled", "debug_mode"}},
        {"paths", {"data_dir", "backup_dir", "reports_dir"}},
        {"limits", {"critical_autonomy_months", "warning_autonomy_months", "expiration_critical_days", "expiration_warning_days"}}
    };

    std::map<std::string, std::map<std::string, std::string>> values;
    std::string currentSection;
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        const std::string cleanedLine = trim(line);
        if (cleanedLine.empty() || cleanedLine.front() == '#' || cleanedLine.front() == ';') {
            continue;
        }

        if (cleanedLine.front() == '[') {
            if (cleanedLine.back() != ']' || cleanedLine.size() <= 2) {
                return {false, "arquivo corrompido na linha " + std::to_string(lineNumber)};
            }

            currentSection = trim(cleanedLine.substr(1, cleanedLine.size() - 2));
            if (allowedSections.count(currentSection) == 0) {
                return {false, "secao invalida: " + currentSection};
            }
            continue;
        }

        const std::size_t separator = cleanedLine.find('=');
        if (separator == std::string::npos || separator == 0) {
            return {false, "arquivo corrompido na linha " + std::to_string(lineNumber)};
        }

        if (currentSection.empty()) {
            return {false, "arquivo corrompido: chave fora de secao na linha " + std::to_string(lineNumber)};
        }

        const std::string key = trim(cleanedLine.substr(0, separator));
        const std::string value = trim(cleanedLine.substr(separator + 1));
        const auto allowedSectionKeys = allowedKeys.find(currentSection);
        if (allowedSectionKeys == allowedKeys.end() || allowedSectionKeys->second.count(key) == 0) {
            return {false, "chave invalida: " + currentSection + "." + key};
        }

        if (values[currentSection].count(key) > 0) {
            return {false, "chave duplicada: " + currentSection + "." + key};
        }

        values[currentSection][key] = value;
    }

    AppConfig config;
    std::string rawValue;
    std::string message;

    if (!requireKey(values, "system", "version", config.system.version, message) ||
        !requireKey(values, "system", "currency", config.system.currency, message) ||
        !requireKey(values, "paths", "data_dir", config.paths.dataDir, message) ||
        !requireKey(values, "paths", "backup_dir", config.paths.backupDir, message) ||
        !requireKey(values, "paths", "reports_dir", config.paths.reportsDir, message)) {
        return {false, message};
    }

    if (config.system.version.empty() || config.system.currency.empty() ||
        config.paths.dataDir.empty() || config.paths.backupDir.empty() || config.paths.reportsDir.empty()) {
        return {false, "valor invalido: campos textuais nao podem ser vazios"};
    }

    if (!requireKey(values, "system", "logs_enabled", rawValue, message)) {
        return {false, message};
    }

    if (!parseBool(rawValue, config.system.logsEnabled)) {
        return {false, "valor invalido: system.logs_enabled"};
    }

    if (!requireKey(values, "system", "backup_enabled", rawValue, message)) {
        return {false, message};
    }

    if (!parseBool(rawValue, config.system.backupEnabled)) {
        return {false, "valor invalido: system.backup_enabled"};
    }

    if (!requireKey(values, "system", "reports_enabled", rawValue, message)) {
        return {false, message};
    }

    if (!parseBool(rawValue, config.system.reportsEnabled)) {
        return {false, "valor invalido: system.reports_enabled"};
    }

    if (!requireKey(values, "system", "debug_mode", rawValue, message)) {
        return {false, message};
    }

    if (!parseBool(rawValue, config.system.debugMode)) {
        return {false, "valor invalido: system.debug_mode"};
    }

    if (!requireKey(values, "limits", "critical_autonomy_months", rawValue, message)) {
        return {false, message};
    }

    if (!parseDouble(rawValue, config.limits.criticalAutonomyMonths) ||
        config.limits.criticalAutonomyMonths <= 0.0) {
        return {false, "valor invalido: limits.critical_autonomy_months"};
    }

    if (!requireKey(values, "limits", "warning_autonomy_months", rawValue, message)) {
        return {false, message};
    }

    if (!parseDouble(rawValue, config.limits.warningAutonomyMonths) ||
        config.limits.warningAutonomyMonths <= 0.0 ||
        config.limits.warningAutonomyMonths < config.limits.criticalAutonomyMonths) {
        return {false, "valor invalido: limits.warning_autonomy_months"};
    }

    if (!requireKey(values, "limits", "expiration_critical_days", rawValue, message)) {
        return {false, message};
    }

    if (!parseInt(rawValue, config.limits.expirationCriticalDays) ||
        config.limits.expirationCriticalDays <= 0) {
        return {false, "valor invalido: limits.expiration_critical_days"};
    }

    if (!requireKey(values, "limits", "expiration_warning_days", rawValue, message)) {
        return {false, message};
    }

    if (!parseInt(rawValue, config.limits.expirationWarningDays) ||
        config.limits.expirationWarningDays <= 0 ||
        config.limits.expirationWarningDays < config.limits.expirationCriticalDays) {
        return {false, "valor invalido: limits.expiration_warning_days"};
    }

    return {true, "Config carregada com sucesso.", config};
}
