#include "ExpirationAnalyzer.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../csv/CsvInventoryReader.h"
#include "../logging/Logger.h"

ExpirationAnalysisResult ExpirationAnalyzer::analyze(const std::filesystem::path &inventoryCsvPath) const
{
    CsvInventoryReader reader;
    const CsvReadResult readResult = reader.read(inventoryCsvPath);
    if (!readResult.success) {
        Logger::log(LogLevel::Error, "ExpirationAnalyzer", "Falha ao ler estoque para validade: " + readResult.message);
        return {false, readResult.message};
    }

    ExpirationAnalysisResult result;
    result.success = true;
    result.message = "Relatorio de validade gerado com sucesso.";

    for (const InventoryItem &item : readResult.items) {
        if (item.quantity < 0.0) {
            Logger::log(LogLevel::Error, "ExpirationAnalyzer", "Quantidade negativa para item: " + item.name);
            return {false, "quantidade negativa para o item: " + item.name};
        }

        if (item.expirationDate.empty()) {
            Logger::log(LogLevel::Error, "ExpirationAnalyzer", "Validade vazia para item: " + item.name);
            return {false, "validade vazia para o item: " + item.name};
        }

        ExpirationItem expirationItem;
        expirationItem.item = item.name;
        expirationItem.category = item.category;
        expirationItem.quantity = item.quantity;
        expirationItem.expirationDate = item.expirationDate;

        if (item.expirationDate == "sem_validade") {
            expirationItem.remainingDays = 0;
            expirationItem.status = "SEM_VALIDADE";
            result.items.push_back(expirationItem);
            continue;
        }

        std::tm parsedDate = {};
        if (!parseDate(item.expirationDate, parsedDate)) {
            Logger::log(LogLevel::Error, "ExpirationAnalyzer", "Formato invalido de validade para item: " + item.name);
            return {false, "formato invalido de validade para o item: " + item.name};
        }

        expirationItem.remainingDays = daysUntil(parsedDate);
        expirationItem.status = classify(expirationItem.remainingDays, item.expirationDate);
        result.items.push_back(expirationItem);
    }

    Logger::log(LogLevel::Info, "ExpirationAnalyzer", "Relatorio de validade gerado com sucesso.");
    return result;
}

bool ExpirationAnalyzer::parseDate(const std::string &value, std::tm &date)
{
    std::istringstream input(value);
    input >> std::get_time(&date, "%Y-%m-%d");
    if (input.fail() || !input.eof()) {
        return false;
    }

    const int expectedYear = date.tm_year;
    const int expectedMonth = date.tm_mon;
    const int expectedDay = date.tm_mday;
    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    date.tm_isdst = -1;

    std::tm normalized = date;
    if (std::mktime(&normalized) == -1) {
        return false;
    }

    return normalized.tm_year == expectedYear &&
           normalized.tm_mon == expectedMonth &&
           normalized.tm_mday == expectedDay;
}

int ExpirationAnalyzer::daysUntil(const std::tm &date)
{
    std::tm target = date;
    target.tm_isdst = -1;
    const std::time_t targetTime = std::mktime(&target);

    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm today = *std::localtime(&nowTime);
    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    today.tm_isdst = -1;
    const std::time_t todayTime = std::mktime(&today);

    return static_cast<int>((targetTime - todayTime) / 86400);
}

std::string ExpirationAnalyzer::classify(int remainingDays, const std::string &expirationDate)
{
    if (expirationDate == "sem_validade") {
        return "SEM_VALIDADE";
    }

    if (remainingDays < 0) {
        return "VENCIDO";
    }

    if (remainingDays <= 30) {
        return "CRÍTICO";
    }

    if (remainingDays <= 90) {
        return "ATENÇÃO";
    }

    return "OK";
}
