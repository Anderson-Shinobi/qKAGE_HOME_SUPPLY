#include "StockController.h"

#include <functional>
#include <iostream>
#include <sstream>

namespace {
ControllerResult invalidArguments()
{
    return {false, "argumentos invalidos", "INVALID_COMMAND", ""};
}

ControllerResult runCaptured(int argc, char *argv[], int minArgs, int maxArgs, const std::function<int(int, char **)> &operation)
{
    if (argc < minArgs || argc > maxArgs) {
        return invalidArguments();
    }

    std::ostringstream output;
    std::ostringstream errors;
    std::streambuf *oldOutput = std::cout.rdbuf(output.rdbuf());
    std::streambuf *oldErrors = std::cerr.rdbuf(errors.rdbuf());
    const int status = operation(argc, argv);
    std::cout.rdbuf(oldOutput);
    std::cerr.rdbuf(oldErrors);

    return {
        status == 0,
        errors.str(),
        status == 0 ? "" : "SERVICE_ERROR",
        output.str()
    };
}
}

ControllerResult StockController::list(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 1, 3, [this](int callArgc, char **callArgv) { return service_.list(callArgc, callArgv); });
}

ControllerResult StockController::add(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 10, 11, [this](int callArgc, char **callArgv) { return service_.add(callArgc, callArgv); });
}

ControllerResult StockController::edit(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 11, 12, [this](int callArgc, char **callArgv) { return service_.edit(callArgc, callArgv); });
}

ControllerResult StockController::remove(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 3, 4, [this](int callArgc, char **callArgv) { return service_.remove(callArgc, callArgv); });
}

ControllerResult StockController::consume(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 5, 6, [this](int callArgc, char **callArgv) { return service_.consume(callArgc, callArgv); });
}

ControllerResult StockController::autonomy(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.autonomy(callArgc, callArgv); });
}

ControllerResult StockController::shoppingList(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.shoppingList(callArgc, callArgv); });
}

ControllerResult StockController::expirationReport(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.expirationReport(callArgc, callArgv); });
}

ControllerResult StockController::rotationAdvice(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.rotationAdvice(callArgc, callArgv); });
}

ControllerResult StockController::historyReport(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.historyReport(callArgc, callArgv); });
}
