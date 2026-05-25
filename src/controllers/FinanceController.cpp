#include "FinanceController.h"

#include <functional>
#include <iostream>
#include <sstream>

namespace {
ControllerResult runCaptured(int argc, char *argv[], int minArgs, int maxArgs, const std::function<int(int, char **)> &operation)
{
    if (argc < minArgs || argc > maxArgs) {
        return {false, "argumentos invalidos", "INVALID_COMMAND", ""};
    }

    std::ostringstream output;
    std::ostringstream errors;
    std::streambuf *oldOutput = std::cout.rdbuf(output.rdbuf());
    std::streambuf *oldErrors = std::cerr.rdbuf(errors.rdbuf());
    const int status = operation(argc, argv);
    std::cout.rdbuf(oldOutput);
    std::cerr.rdbuf(oldErrors);

    return {status == 0, errors.str(), status == 0 ? "" : "SERVICE_ERROR", output.str()};
}
}

ControllerResult FinanceController::savings(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 6, 6, [this](int callArgc, char **callArgv) { return service_.savings(callArgc, callArgv); });
}

ControllerResult FinanceController::invest(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 4, 5, [this](int callArgc, char **callArgv) { return service_.invest(callArgc, callArgv); });
}

ControllerResult FinanceController::compound(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 5, 6, [this](int callArgc, char **callArgv) { return service_.compound(callArgc, callArgv); });
}

ControllerResult FinanceController::piggyAdd(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 6, 7, [this](int callArgc, char **callArgv) { return service_.piggyAdd(callArgc, callArgv); });
}

ControllerResult FinanceController::piggyReport(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.piggyReport(callArgc, callArgv); });
}
