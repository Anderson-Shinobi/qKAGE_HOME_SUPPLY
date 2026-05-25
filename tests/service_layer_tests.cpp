#include "cli/CommandDispatcher.h"
#include "services/FinanceService.h"
#include "services/ReportService.h"
#include "services/StockService.h"
#include "services/SystemService.h"

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

std::vector<char *> argvFrom(std::vector<std::string> &args)
{
    std::vector<char *> argv;
    for (std::string &arg : args) {
        argv.push_back(arg.data());
    }
    return argv;
}

int runStockAdd(const StockService &service, const std::filesystem::path &stockCsv)
{
    std::vector<std::string> args = {
        "test", "add", "Servico arroz", "Alimentos", "2", "kg", "1", "7.5", "2027-12-31", "1", stockCsv.string()
    };
    std::vector<char *> argv = argvFrom(args);
    return service.add(static_cast<int>(argv.size()), argv.data());
}
}

int main()
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "qkage_service_layer_tests";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const std::filesystem::path logPath = root / "logs" / "qkage.log";
    setenv("QKAGE_LOG_PATH", logPath.string().c_str(), 1);

    const std::filesystem::path stockCsv = root / "estoque.csv";
    const std::filesystem::path piggyCsv = root / "piggybanks.csv";
    const std::filesystem::path reportsDir = root / "reports";

    StockService stockService;
    if (runStockAdd(stockService, stockCsv) != 0) {
        fail("StockService add falhou");
    }

    {
        std::vector<std::string> args = {"test", "add", "Servico feijao", "Alimentos", "3", "kg", "1", "8.5", "2027-12-31", "1", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        if (stockService.add(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("StockService add para remocao falhou");
        }
    }

    {
        std::vector<std::string> args = {"test", "remove", "Servico feijao", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        if (stockService.remove(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("StockService remove valido falhou");
        }
    }

    {
        std::vector<std::string> args = {"test", "remove", "Servico feijao", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        if (stockService.remove(static_cast<int>(argv.size()), argv.data()) == 0) {
            fail("StockService remove deveria rejeitar item inexistente");
        }
    }

    {
        std::vector<std::string> args = {"test", "analyze", stockCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        if (stockService.autonomy(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("StockService autonomy falhou");
        }
    }

    FinanceService financeService;
    {
        std::vector<std::string> args = {"test", "savings", "Creme dental", "3", "8.00", "6.00"};
        std::vector<char *> argv = argvFrom(args);
        if (financeService.savings(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("FinanceService savings falhou");
        }
    }

    std::ofstream piggyFile(piggyCsv);
    piggyFile << "nome,valor_atual,meta,aporte_mensal,status\n";
    piggyFile.close();

    ReportService reportService;
    {
        std::vector<std::string> args = {"test", "monthly-report", stockCsv.string(), piggyCsv.string()};
        std::vector<char *> argv = argvFrom(args);
        if (reportService.monthlyReport(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("ReportService monthlyReport falhou");
        }
    }

    {
        std::vector<std::string> args = {"test", "export-report", stockCsv.string(), piggyCsv.string(), reportsDir.string()};
        std::vector<char *> argv = argvFrom(args);
        if (reportService.exportReport(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("ReportService exportReport falhou");
        }
    }

    SystemService systemService;
    {
        std::vector<std::string> args = {"test", "error-codes"};
        std::vector<char *> argv = argvFrom(args);
        if (systemService.errorCodes(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("SystemService errorCodes falhou");
        }
    }

    CommandDispatcher dispatcher;
    {
        std::vector<std::string> args = {"test", "help"};
        std::vector<char *> argv = argvFrom(args);
        if (dispatcher.dispatch(static_cast<int>(argv.size()), argv.data()) != 0) {
            fail("CommandDispatcher help falhou");
        }
    }

    return 0;
}
