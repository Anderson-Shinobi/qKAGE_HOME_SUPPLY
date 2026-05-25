#pragma once

class SystemService {
public:
    int backup(int argc, char *argv[]) const;
    int integrityCheck(int argc, char *argv[]) const;
    int configShow(int argc, char *argv[]) const;
    int logsShow(int argc, char *argv[]) const;
    int errorCodes(int argc, char *argv[]) const;
};
