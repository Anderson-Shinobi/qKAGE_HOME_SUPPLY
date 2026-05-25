#pragma once

#include <string>

struct ControllerResult {
    bool success = false;
    std::string message;
    std::string error_code;
    std::string payload;
};
