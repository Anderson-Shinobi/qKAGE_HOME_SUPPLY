#pragma once

#include "ControllerResult.h"
#include "../services/ReportService.h"

class ReportController {
public:
    ControllerResult monthlyReport(int argc, char *argv[]) const;
    ControllerResult exportReport(int argc, char *argv[]) const;

private:
    ReportService service_;
};
