#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "../controllers/FinanceController.h"
#include "../controllers/ReportController.h"
#include "../controllers/StockController.h"
#include "../controllers/SystemController.h"

struct CommandSpec {
    std::string name;
    std::string description;
    std::string example;
    int minArgs = 0;
    int maxArgs = 0;
    std::function<int(int, char **)> handler;
};

class CommandDispatcher {
public:
    CommandDispatcher();

    int dispatch(int argc, char *argv[]) const;
    int printHelp(const char *programName) const;

private:
    std::map<std::string, CommandSpec> commands_;
    StockController stockController_;
    FinanceController financeController_;
    ReportController reportController_;
    SystemController systemController_;

    void registerCommand(
        const std::string &name,
        const std::string &description,
        const std::string &example,
        int minArgs,
        int maxArgs,
        std::function<int(int, char **)> handler);
};
