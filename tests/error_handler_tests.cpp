#include "core/ErrorHandler.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
bool contains(const std::string &content, const std::string &expected)
{
    return content.find(expected) != std::string::npos;
}

std::string readFile(const std::filesystem::path &path)
{
    std::ifstream file(path);
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

void fail(const std::string &message)
{
    std::cerr << "Falha: " << message << '\n';
    std::exit(1);
}
}

int main()
{
    const std::filesystem::path logPath = std::filesystem::temp_directory_path() / "qkage_error_handler_tests" / "qkage.log";
    std::filesystem::remove_all(logPath.parent_path());
    std::filesystem::create_directories(logPath.parent_path());
    setenv("QKAGE_LOG_PATH", logPath.string().c_str(), 1);

    const SystemError info = ErrorHandler::createCustom(
        "TEST_INFO",
        ErrorSeverity::Info,
        "ErrorHandlerTests",
        "Info criada.",
        "Nenhuma acao necessaria.");
    if (info.severity != ErrorSeverity::Info || ErrorHandler::severityToString(info.severity) != "INFO") {
        fail("erro INFO nao foi criado corretamente");
    }

    const SystemError warning = ErrorHandler::createCustom(
        "TEST_WARNING",
        ErrorSeverity::Warning,
        "ErrorHandlerTests",
        "Warning criada.",
        "Verificar entrada.");
    if (warning.severity != ErrorSeverity::Warning || ErrorHandler::severityToString(warning.severity) != "WARNING") {
        fail("erro WARNING nao foi criado corretamente");
    }

    const SystemError error = ErrorHandler::create(
        "INVALID_NUMBER",
        ErrorSeverity::Error,
        "ErrorHandlerTests",
        "campo quantidade");
    if (error.severity != ErrorSeverity::Error || error.code != "INVALID_NUMBER") {
        fail("erro ERROR nao foi criado corretamente");
    }

    const SystemError critical = ErrorHandler::createCustom(
        "TEST_CRITICAL",
        ErrorSeverity::Critical,
        "ErrorHandlerTests",
        "Critical criada.",
        "Interromper fluxo.");
    if (critical.severity != ErrorSeverity::Critical || ErrorHandler::severityToString(critical.severity) != "CRITICAL") {
        fail("erro CRITICAL nao foi criado corretamente");
    }

    const std::string logContent = readFile(logPath);
    if (!contains(logContent, "[ERROR]") ||
        !contains(logContent, "INVALID_NUMBER") ||
        !contains(logContent, "CRITICAL TEST_CRITICAL")) {
        fail("integracao com Logger nao registrou ERROR e CRITICAL");
    }

    bool foundInvalidConfig = false;
    for (const ErrorCodeInfo &infoEntry : ErrorHandler::catalog()) {
        if (infoEntry.code == "INVALID_CONFIG" &&
            contains(infoEntry.description, "configuracao") &&
            !infoEntry.recommendedAction.empty()) {
            foundInvalidConfig = true;
            break;
        }
    }

    if (!foundInvalidConfig) {
        fail("catalogo de codigos nao contem INVALID_CONFIG");
    }

    return 0;
}
