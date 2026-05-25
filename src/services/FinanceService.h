#pragma once

class FinanceService {
public:
    int savings(int argc, char *argv[]) const;
    int invest(int argc, char *argv[]) const;
    int compound(int argc, char *argv[]) const;
    int piggyAdd(int argc, char *argv[]) const;
    int piggyReport(int argc, char *argv[]) const;
};
