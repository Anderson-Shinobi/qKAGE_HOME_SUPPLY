#pragma once

#include "ControllerResult.h"
#include "../services/StockService.h"

class StockController {
public:
    ControllerResult list(int argc, char *argv[]) const;
    ControllerResult add(int argc, char *argv[]) const;
    ControllerResult edit(int argc, char *argv[]) const;
    ControllerResult remove(int argc, char *argv[]) const;
    ControllerResult consume(int argc, char *argv[]) const;
    ControllerResult autonomy(int argc, char *argv[]) const;
    ControllerResult shoppingList(int argc, char *argv[]) const;
    ControllerResult expirationReport(int argc, char *argv[]) const;
    ControllerResult rotationAdvice(int argc, char *argv[]) const;
    ControllerResult historyReport(int argc, char *argv[]) const;

private:
    StockService service_;
};
