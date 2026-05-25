#pragma once

#include <string>
#include <vector>

enum class ErrorSeverity {
    Info,
    Warning,
    Error,
    Critical
};

struct SystemError {
    std::string code;
    ErrorSeverity severity = ErrorSeverity::Info;
    std::string module;
    std::string message;
    std::string correctionSuggestion;
};

struct ErrorCodeInfo {
    std::string code;
    std::string description;
    std::string recommendedAction;
};

class ErrorHandler {
public:
    static SystemError create(
        const std::string &code,
        ErrorSeverity severity,
        const std::string &module,
        const std::string &details = "");

    static SystemError createCustom(
        const std::string &code,
        ErrorSeverity severity,
        const std::string &module,
        const std::string &message,
        const std::string &correctionSuggestion);

    static const std::vector<ErrorCodeInfo> &catalog();
    static std::string severityToString(ErrorSeverity severity);
};
