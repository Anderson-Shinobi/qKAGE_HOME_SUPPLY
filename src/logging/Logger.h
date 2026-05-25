#pragma once

#include <filesystem>
#include <string>
#include <vector>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

struct LogReadResult {
    bool success = false;
    std::string message;
    std::vector<std::string> lines;
};

class Logger {
public:
    static bool log(LogLevel level, const std::string &module, const std::string &message);
    static std::filesystem::path logPath();
    static LogReadResult tail(const std::filesystem::path &logPath, int lineCount, const std::string &levelFilter = "");
    static bool isValidLevel(const std::string &level);
    static std::string levelToString(LogLevel level);

private:
    static bool shouldWrite(LogLevel level);
    static bool rotateIfNeeded(const std::filesystem::path &logPath);
};
