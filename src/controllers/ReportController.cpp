#include "ReportController.h"

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

ControllerResult ReportController::monthlyReport(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 4, [this](int callArgc, char **callArgv) { return service_.monthlyReport(callArgc, callArgv); });
}

ControllerResult ReportController::exportReport(int argc, char *argv[]) const
{
    return runCaptured(argc, argv, 2, 5, [this](int callArgc, char **callArgv) { return service_.exportReport(callArgc, callArgv); });
}
