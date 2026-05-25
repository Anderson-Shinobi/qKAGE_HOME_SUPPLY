#include "cli/CommandDispatcher.h"
#include "controllers/FinanceController.h"
#include "controllers/ReportController.h"
#include "controllers/StockController.h"
#include "controllers/SystemController.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
void fail(const std::string &message)
{
    std::cerr << "Falha: " << message << '\n';
    std::exit(1);
}

bool contains(const std::string &content, const std::string &expected)
{
    return content.find(expected) != std::string::npos;
}

std::vector<char *> argvFrom(std::vector<std::string> &args)
{
    std::vector<char *> argv;
    for (std::string &arg : args) {
        argv.push_back(arg.data());
    }
    return argv;
}
}

int main()
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "qkage_controller_tests";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const std::filesystem::path logPath = root / "logs" / "qkage.log";
    setenv("QKAGE_LOG_PATH", logPath.string().c_str(), 1);

    const std::filesystem::path stockCsv = root / "estoque.csv";
    StockController stockController;
    {
        std::vector<std::string> args = {"test", "add", "Controller arroz", "Alimentos", "2", "kg", "1", "7.5", "2027-12-31", "1", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = stockController.add(static_cast<int>(argv.size()), argv.data());
        if (!result.success || !contains(result.payload, "Item adicionado ao estoque")) {
            fail("StockController sucesso nao retornou dados formatados");
        }
    }

    {
        std::vector<std::string> args = {"test", "add", "", "Alimentos", "2", "kg", "1", "7.5", "2027-12-31", "1", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = stockController.add(static_cast<int>(argv.size()), argv.data());
        if (result.success || result.error_code != "SERVICE_ERROR" || !contains(result.message, "item e obrigatorio")) {
            fail("StockController erro nao foi propagado");
        }
    }

    {
        std::vector<std::string> args = {"test", "remove", "Controller arroz", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = stockController.remove(static_cast<int>(argv.size()), argv.data());
        if (!result.success || !contains(result.payload, "Item removido do estoque")) {
            fail("StockController remove valido falhou");
        }
    }

    {
        std::vector<std::string> args = {"test", "history-report", (root / "stock_movements.csv").string()};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = stockController.historyReport(static_cast<int>(argv.size()), argv.data());
        if (!result.success || !contains(result.payload, "ADJUST") || !contains(result.payload, "Item removido do estoque")) {
            fail("StockController remove nao registrou historico ADJUST");
        }
    }

    {
        std::vector<std::string> args = {"test", "remove", "Inexistente", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = stockController.remove(static_cast<int>(argv.size()), argv.data());
        if (result.success || result.error_code != "SERVICE_ERROR" || !contains(result.message, "item nao encontrado")) {
            fail("StockController remove nao validou item inexistente");
        }
    }

    FinanceController financeController;
    {
        std::vector<std::string> args = {"test", "savings", "Item", "3", "8.00", "6.00"};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = financeController.savings(static_cast<int>(argv.size()), argv.data());
        if (!result.success || !contains(result.payload, "Economia total")) {
            fail("FinanceController sucesso nao retornou dados formatados");
        }
    }

    {
        std::vector<std::string> args = {"test", "savings", "Item", "0", "8.00", "6.00"};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = financeController.savings(static_cast<int>(argv.size()), argv.data());
        if (result.success || result.error_code != "SERVICE_ERROR" || !contains(result.message, "quantidade deve ser maior que zero")) {
            fail("FinanceController erro de service nao foi propagado");
        }
    }

    const std::filesystem::path piggyCsv = root / "piggybanks.csv";
    std::ofstream piggyFile(piggyCsv);
    piggyFile << "nome,valor_atual,meta,aporte_mensal,status\n";
    piggyFile.close();

    ReportController reportController;
    {
        std::vector<std::string> args = {"test", "monthly-report", stockCsv.string(), piggyCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = reportController.monthlyReport(static_cast<int>(argv.size()), argv.data());
        if (!result.success || !contains(result.payload, "[ESTOQUE]")) {
            fail("ReportController sucesso nao retornou relatorio");
        }
    }

    SystemController systemController;
    {
        std::vector<std::string> args = {"test", "error-codes"};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = systemController.errorCodes(static_cast<int>(argv.size()), argv.data());
        if (!result.success || !contains(result.payload, "CSV_NOT_FOUND")) {
            fail("SystemController sucesso nao retornou catalogo");
        }
    }

    {
        std::vector<std::string> args = {"test", "logs-show", "ERROR", "extra"};
        std::vector<char *> argv = argvFrom(args);
        const ControllerResult result = systemController.logsShow(static_cast<int>(argv.size()), argv.data());
        if (result.success || result.error_code != "INVALID_COMMAND") {
            fail("SystemController argumentos invalidos nao foram rejeitados");
        }
    }

    CommandDispatcher dispatcher;
    {
        std::vector<std::string> args = {"test", "error-codes"};
        std::vector<char *> argv = argvFrom(args);
        if (dispatcher.dispatch(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("CommandDispatcher nao executou via controller");
        }
    }

    return 0;
}
