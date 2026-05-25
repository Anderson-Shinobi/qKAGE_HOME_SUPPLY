#pragma once

class StockService {
public:
    int list(int argc, char *argv[]) const;
    int add(int argc, char *argv[]) const;
    int edit(int argc, char *argv[]) const;
    int remove(int argc, char *argv[]) const;
    int consume(int argc, char *argv[]) const;
    int autonomy(int argc, char *argv[]) const;
    int shoppingList(int argc, char *argv[]) const;
    int expirationReport(int argc, char *argv[]) const;
    int rotationAdvice(int argc, char *argv[]) const;
    int historyReport(int argc, char *argv[]) const;
};
