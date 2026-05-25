#include "CommandDispatcher.h"

#include <iomanip>
#include <iostream>

#include "../core/ErrorHandler.h"
#include "../logging/Logger.h"

namespace {
int emitResult(const ControllerResult &result)
{
    if (!result.payload.empty()) {
        std::cout << result.payload;
    }

    if (!result.message.empty()) {
        std::cerr << result.message;
    }

    return result.success ? 0 : 1;
}

int reportInvalidCommand(const std::string &module, const std::string &details)
{
    const SystemError error = ErrorHandler::create(
        "INVALID_COMMAND",
        ErrorSeverity::Error,
        module,
        details);
    std::cerr << "Erro [" << error.code << "]: " << error.message << '\n'
              << "Sugestao: " << error.correctionSuggestion << '\n';
    return 1;
}
}

CommandDispatcher::CommandDispatcher()
{
    registerCommand("help", "Lista comandos disponiveis.", "help", 2, 2, [this](int argc, char **argv) {
        (void)argc;
        return printHelp(argv[0]);
    });

    registerCommand("list", "Lista itens do estoque.", "list data/estoque.csv", 2, 3, [this](int argc, char **argv) { return emitResult(stockController_.list(argc, argv)); });
    registerCommand("analyze", "Analisa autonomia do estoque.", "analyze data/estoque.csv", 2, 3, [this](int argc, char **argv) { return emitResult(stockController_.autonomy(argc, argv)); });
    registerCommand("add", "Adiciona item ao estoque.", "add \"Arroz\" \"Alimentos\" 5 \"kg\" 1 7.50 2027-12-31 1", 10, 11, [this](int argc, char **argv) { return emitResult(stockController_.add(argc, argv)); });
    registerCommand("edit", "Edita item do estoque.", "edit \"Arroz\" \"Arroz\" \"Alimentos\" 5 \"kg\" 1 7.50 2027-12-31 1", 11, 12, [this](int argc, char **argv) { return emitResult(stockController_.edit(argc, argv)); });
    registerCommand("remove", "Remove item do estoque.", "remove \"Arroz\"", 3, 4, [this](int argc, char **argv) { return emitResult(stockController_.remove(argc, argv)); });
    registerCommand("consume", "Registra consumo de item.", "consume \"Arroz\" 1 \"uso mensal\"", 5, 6, [this](int argc, char **argv) { return emitResult(stockController_.consume(argc, argv)); });
    registerCommand("savings", "Calcula economia por compra.", "savings \"Creme dental\" 18 8.00 5.90", 6, 6, [this](int argc, char **argv) { return emitResult(financeController_.savings(argc, argv)); });
    registerCommand("invest", "Calcula capital liberado para investimento.", "invest 40.60 12 0", 4, 5, [this](int argc, char **argv) { return emitResult(financeController_.invest(argc, argv)); });
    registerCommand("compound", "Projeta investimento com juros compostos.", "compound 40.60 0.008 120 0", 5, 6, [this](int argc, char **argv) { return emitResult(financeController_.compound(argc, argv)); });
    registerCommand("piggy-add", "Cadastra cofrinho estrategico.", "piggy-add \"Gas\" 45.00 120.00 15.00", 6, 7, [this](int argc, char **argv) { return emitResult(financeController_.piggyAdd(argc, argv)); });
    registerCommand("piggy-report", "Exibe relatorio de cofrinhos.", "piggy-report", 2, 3, [this](int argc, char **argv) { return emitResult(financeController_.piggyReport(argc, argv)); });
    registerCommand("monthly-report", "Gera relatorio mensal consolidado.", "monthly-report", 2, 4, [this](int argc, char **argv) { return emitResult(reportController_.monthlyReport(argc, argv)); });
    registerCommand("export-report", "Exporta relatorio mensal para Markdown.", "export-report", 2, 5, [this](int argc, char **argv) { return emitResult(reportController_.exportReport(argc, argv)); });
    registerCommand("backup", "Cria backup versionado dos dados.", "backup", 2, 4, [this](int argc, char **argv) { return emitResult(systemController_.backup(argc, argv)); });
    registerCommand("integrity-check", "Verifica integridade dos dados.", "integrity-check --fix", 2, 4, [this](int argc, char **argv) { return emitResult(systemController_.integrityCheck(argc, argv)); });
    registerCommand("history-report", "Exibe historico de movimentacoes.", "history-report", 2, 3, [this](int argc, char **argv) { return emitResult(stockController_.historyReport(argc, argv)); });
    registerCommand("shopping-list", "Gera lista automatica de compras.", "shopping-list", 2, 3, [this](int argc, char **argv) { return emitResult(stockController_.shoppingList(argc, argv)); });
    registerCommand("expiration-report", "Analisa validade e vencimento.", "expiration-report", 2, 3, [this](int argc, char **argv) { return emitResult(stockController_.expirationReport(argc, argv)); });
    registerCommand("rotation-advice", "Recomenda rotacao de estoque.", "rotation-advice", 2, 3, [this](int argc, char **argv) { return emitResult(stockController_.rotationAdvice(argc, argv)); });
    registerCommand("config-show", "Exibe configuracao central.", "config-show", 2, 3, [this](int argc, char **argv) { return emitResult(systemController_.configShow(argc, argv)); });
    registerCommand("logs-show", "Exibe ultimas linhas do log.", "logs-show ERROR", 2, 3, [this](int argc, char **argv) { return emitResult(systemController_.logsShow(argc, argv)); });
    registerCommand("error-codes", "Lista codigos de erro.", "error-codes", 2, 2, [this](int argc, char **argv) { return emitResult(systemController_.errorCodes(argc, argv)); });
}

int CommandDispatcher::dispatch(int argc, char *argv[]) const
{
    if (argc <= 1) {
        Logger::log(LogLevel::Info, "CommandDispatcher", "Comando help acionado sem argumentos.");
        return printHelp(argv[0]);
    }

    const std::string commandName = argv[1];
    const auto commandIt = commands_.find(commandName);
    if (commandIt == commands_.end()) {
        return reportInvalidCommand("CommandDispatcher", "comando recebido: " + commandName);
    }

    const CommandSpec &command = commandIt->second;
    if (argc < command.minArgs) {
        return reportInvalidCommand("CommandDispatcher", "argumentos insuficientes para comando: " + commandName);
    }

    if (argc > command.maxArgs) {
        return reportInvalidCommand("CommandDispatcher", "argumentos invalidos para comando: " + commandName);
    }

    Logger::log(LogLevel::Info, "CommandDispatcher", "Executando comando: " + commandName);
    return command.handler(argc, argv);
}

int CommandDispatcher::printHelp(const char *programName) const
{
    std::cout << "KAGE Home Supply - Ajuda da CLI\n";
    std::cout << "Uso: " << programName << " <comando> [argumentos]\n\n";
    std::cout << std::left
              << std::setw(20) << "comando"
              << std::setw(44) << "descricao"
              << "exemplo\n";

    for (const auto &entry : commands_) {
        const CommandSpec &command = entry.second;
        std::cout << std::left
                  << std::setw(20) << command.name
                  << std::setw(44) << command.description
                  << programName << ' ' << command.example << '\n';
    }

    return 0;
}

void CommandDispatcher::registerCommand(
    const std::string &name,
    const std::string &description,
    const std::string &example,
    int minArgs,
    int maxArgs,
    std::function<int(int, char **)> handler)
{
    CommandSpec command;
    command.name = name;
    command.description = description;
    command.example = example;
    command.minArgs = minArgs;
    command.maxArgs = maxArgs;
    command.handler = handler;
    commands_[name] = command;
}
