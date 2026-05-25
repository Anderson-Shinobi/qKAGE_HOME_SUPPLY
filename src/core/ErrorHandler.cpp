#include "ErrorHandler.h"

#include <map>

#include "../logging/Logger.h"

namespace {
struct ErrorTemplate {
    std::string message;
    std::string suggestion;
};

const std::map<std::string, ErrorTemplate> &templates()
{
    static const std::map<std::string, ErrorTemplate> values = {
        {"CSV_NOT_FOUND", {"CSV inexistente.", "Verifique o caminho informado ou crie o arquivo CSV esperado."}},
        {"CSV_CORRUPTED", {"CSV corrompido.", "Confira cabecalho, colunas obrigatorias e linhas malformadas."}},
        {"REQUIRED_FIELD_EMPTY", {"Campo obrigatorio vazio.", "Preencha o campo indicado antes de executar novamente."}},
        {"INVALID_NUMBER", {"Valor numerico invalido.", "Use apenas numeros validos nos campos numericos."}},
        {"PERMISSION_DENIED", {"Permissao negada.", "Verifique permissoes de leitura e escrita no caminho informado."}},
        {"INVALID_COMMAND", {"Comando invalido.", "Execute o programa sem argumentos para ver a lista de comandos validos."}},
        {"INVALID_CONFIG", {"Arquivo de configuracao invalido.", "Corrija secoes, chaves obrigatorias e valores em data/config.ini."}}
    };

    return values;
}

void logIfNeeded(const SystemError &error)
{
    if (error.severity == ErrorSeverity::Error) {
        Logger::log(LogLevel::Error, error.module, error.code + ": " + error.message);
        return;
    }

    if (error.severity == ErrorSeverity::Critical) {
        Logger::log(LogLevel::Error, error.module, "CRITICAL " + error.code + ": " + error.message);
    }
}
}

SystemError ErrorHandler::create(
    const std::string &code,
    ErrorSeverity severity,
    const std::string &module,
    const std::string &details)
{
    const auto templateIt = templates().find(code);
    if (templateIt == templates().end()) {
        return createCustom(
            code,
            severity,
            module,
            details.empty() ? "Erro nao catalogado." : details,
            "Consulte a documentacao ou revise a operacao executada.");
    }

    std::string message = templateIt->second.message;
    if (!details.empty()) {
        message += " " + details;
    }

    return createCustom(code, severity, module, message, templateIt->second.suggestion);
}

SystemError ErrorHandler::createCustom(
    const std::string &code,
    ErrorSeverity severity,
    const std::string &module,
    const std::string &message,
    const std::string &correctionSuggestion)
{
    SystemError error;
    error.code = code;
    error.severity = severity;
    error.module = module;
    error.message = message;
    error.correctionSuggestion = correctionSuggestion;

    logIfNeeded(error);
    return error;
}

const std::vector<ErrorCodeInfo> &ErrorHandler::catalog()
{
    static const std::vector<ErrorCodeInfo> values = {
        {"CSV_NOT_FOUND", "CSV inexistente.", "Verifique o caminho informado ou crie o arquivo CSV esperado."},
        {"CSV_CORRUPTED", "CSV corrompido.", "Confira cabecalho, colunas obrigatorias e linhas malformadas."},
        {"REQUIRED_FIELD_EMPTY", "Campo obrigatorio vazio.", "Preencha o campo indicado antes de executar novamente."},
        {"INVALID_NUMBER", "Valor numerico invalido.", "Use apenas numeros validos nos campos numericos."},
        {"PERMISSION_DENIED", "Permissao negada.", "Verifique permissoes de leitura e escrita no caminho informado."},
        {"INVALID_COMMAND", "Comando invalido.", "Execute o programa sem argumentos para ver a lista de comandos validos."},
        {"INVALID_CONFIG", "Arquivo de configuracao invalido.", "Corrija secoes, chaves obrigatorias e valores em data/config.ini."}
    };

    return values;
}

std::string ErrorHandler::severityToString(ErrorSeverity severity)
{
    switch (severity) {
    case ErrorSeverity::Info:
        return "INFO";
    case ErrorSeverity::Warning:
        return "WARNING";
    case ErrorSeverity::Error:
        return "ERROR";
    case ErrorSeverity::Critical:
        return "CRITICAL";
    }

    return "INFO";
}
