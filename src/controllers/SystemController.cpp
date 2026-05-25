#include "SystemController.h"

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

ControllerResult SystemController::backup(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 4, [this](int callArgc, char **callArgv) { return service_.backup(callArgc, callArgv); });
}

ControllerResult SystemController::integrityCheck(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 4, [this](int callArgc, char **callArgv) { return service_.integrityCheck(callArgc, callArgv); });
}

ControllerResult SystemController::configShow(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.configShow(callArgc, callArgv); });
}

ControllerResult SystemController::logsShow(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 3, [this](int callArgc, char **callArgv) { return service_.logsShow(callArgc, callArgv); });
}

ControllerResult SystemController::errorCodes(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 2, [this](int callArgc, char **callArgv) { return service_.errorCodes(callArgc, callArgv); });
}
