#pragma once

#include "ControllerResult.h"
#include "../services/SystemService.h"

class SystemController {
public:
    ControllerResult backup(int argc, char *argv[]) const;
    ControllerResult integrityCheck(int argc, char *argv[]) const;
    ControllerResult configShow(int argc, char *argv[]) const;
    ControllerResult logsShow(int argc, char *argv[]) const;
    ControllerResult errorCodes(int argc, char *argv[]) const;

private:
    SystemService service_;
};
