#pragma once

#include "ControllerResult.h"
#include "../services/FinanceService.h"

class FinanceController {
public:
    ControllerResult savings(int argc, char *argv[]) const;
    ControllerResult invest(int argc, char *argv[]) const;
    ControllerResult compound(int argc, char *argv[]) const;
    ControllerResult piggyAdd(int argc, char *argv[]) const;
    ControllerResult piggyReport(int argc, char *argv[]) const;

private:
    FinanceService service_;
};
