#include "Logger.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../config/ConfigManager.h"

namespace {
constexpr std::uintmax_t kMaxLogSizeBytes = 5 * 1024 * 1024;
const char *kDefaultConfigPath = "data/config.ini";

std::string currentTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    const std::tm localTime = *std::localtime(&time);

    std::ostringstream output;
    output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

bool lineMatchesLevel(const std::string &line, const std::string &level)
{
    if (level.empty()) {
        return true;
    }

    return line.find("[" + level + "]") != std::string::npos;
}
}

bool Logger::log(LogLevel level, const std::string &module, const std::string &message)
{
    if (!shouldWrite(level)) {
        return true;
    }

    const std::filesystem::path logPath = Logger::logPath();
    try {
        if (logPath.has_parent_path()) {
            std::filesystem::create_directories(logPath.parent_path());
        }

        if (!rotateIfNeeded(logPath)) {
            return false;
        }

        std::ofstream file(logPath, std::ios::app);
        if (!file.is_open()) {
            return false;
        }

        file << '[' << currentTimestamp() << "] "
             << '[' << levelToString(level) << "] "
             << '[' << module << "] "
             << message << '\n';
        return file.good();
    } catch (...) {
        return false;
    }
}

LogReadResult Logger::tail(const std::filesystem::path &logPath, int lineCount, const std::string &levelFilter)
{
    if (lineCount <= 0) {
        return {false, "quantidade de linhas invalida"};
    }

    if (!levelFilter.empty() && !isValidLevel(levelFilter)) {
        return {false, "nivel de log invalido: " + levelFilter};
    }

    if (!std::filesystem::exists(logPath)) {
        return {false, "arquivo de log inexistente: " + logPath.string()};
    }

    if (!std::filesystem::is_regular_file(logPath)) {
        return {false, "falha de permissao ao ler log: " + logPath.string()};
    }

    std::ifstream file(logPath);
    if (!file.is_open()) {
        return {false, "falha de permissao ao ler log: " + logPath.string()};
    }

    std::deque<std::string> recentLines;
    std::string line;
    while (std::getline(file, line)) {
        if (!lineMatchesLevel(line, levelFilter)) {
            continue;
        }

        recentLines.push_back(line);
        if (static_cast<int>(recentLines.size()) > lineCount) {
            recentLines.pop_front();
        }
    }

    LogReadResult result;
    result.success = true;
    result.message = "Logs lidos com sucesso.";
    result.lines.assign(recentLines.begin(), recentLines.end());
    return result;
}

bool Logger::isValidLevel(const std::string &level)
{
    return level == "INFO" ||
           level == "WARNING" ||
           level == "ERROR" ||
           level == "DEBUG";
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Debug:
        return "DEBUG";
    }

    return "INFO";
}

std::filesystem::path Logger::logPath()
{
    const char *overridePath = std::getenv("QKAGE_LOG_PATH");
    if (overridePath != nullptr && std::string(overridePath).size() > 0) {
        return overridePath;
    }

    return std::filesystem::path("logs") / "qkage.log";
}

bool Logger::shouldWrite(LogLevel level)
{
    ConfigManager manager;
    const ConfigResult configResult = manager.loadOrCreate(kDefaultConfigPath);
    if (!configResult.success) {
        return level != LogLevel::Debug;
    }

    if (!configResult.config.system.logsEnabled) {
        return false;
    }

    if (level == LogLevel::Debug && !configResult.config.system.debugMode) {
        return false;
    }

    return true;
}

bool Logger::rotateIfNeeded(const std::filesystem::path &logPath)
{
    if (!std::filesystem::exists(logPath)) {
        return true;
    }

    if (std::filesystem::file_size(logPath) <= kMaxLogSizeBytes) {
        return true;
    }

    const std::filesystem::path oldLogPath = logPath.parent_path() / "qkage_old.log";
    if (std::filesystem::exists(oldLogPath)) {
        std::filesystem::remove(oldLogPath);
    }

    std::filesystem::rename(logPath, oldLogPath);
    return true;
}
