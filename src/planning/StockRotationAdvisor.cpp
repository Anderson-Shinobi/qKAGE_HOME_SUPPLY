#include "StockRotationAdvisor.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../core/InventoryItem.h"
#include "../csv/CsvInventoryReader.h"
#include "../logging/Logger.h"

namespace {
int priorityRank(const std::string &priority)
{
    if (priority == "DESCARTAR") {
        return 0;
    }

    if (priority == "CONSUMIR AGORA") {
        return 1;
    }

    if (priority == "CONSUMIR PRIMEIRO") {
        return 2;
    }

    if (priority == "MONITORAR") {
        return 3;
    }

    if (priority == "DADO_INVÁLIDO") {
        return 4;
    }

    return 5;
}

std::string recommendationForPriority(const std::string &priority)
{
    if (priority == "DESCARTAR") {
        return "Descartar produto vencido antes de novo consumo.";
    }

    if (priority == "CONSUMIR AGORA") {
        return "Consumir imediatamente e evitar nova compra ate normalizar.";
    }

    if (priority == "CONSUMIR PRIMEIRO") {
        return "Priorizar no consumo das proximas semanas.";
    }

    if (priority == "MONITORAR") {
        return "Monitorar autonomia e planejar reposicao.";
    }

    if (priority == "DADO_INVÁLIDO") {
        return "Corrigir validade no CSV antes de decidir consumo.";
    }

    return "Manter em rotacao normal.";
}
}

StockRotationResult StockRotationAdvisor::advise(const std::filesystem::path &inventoryCsvPath) const
{
    CsvInventoryReader reader;
    const CsvReadResult readResult = reader.read(inventoryCsvPath);
    if (!readResult.success) {
        Logger::log(LogLevel::Error, "StockRotationAdvisor", "Falha ao ler estoque para rotacao: " + readResult.message);
        return {false, readResult.message};
    }

    StockRotationResult result;
    result.success = true;
    result.message = "Recomendacao de rotacao gerada com sucesso.";

    for (const InventoryItem &item : readResult.items) {
        if (item.quantity < 0.0) {
            Logger::log(LogLevel::Error, "StockRotationAdvisor", "Quantidade negativa para item: " + item.name);
            return {false, "quantidade negativa para o item: " + item.name};
        }

        StockRotationAdvice advice;
        advice.item = item.name;
        advice.category = item.category;
        advice.quantity = item.quantity;
        advice.expirationDate = item.expirationDate;
        advice.autonomyMonths = item.autonomyInMonths();

        if (item.expirationDate != "sem_validade") {
            std::tm parsedDate = {};
            if (!parseDate(item.expirationDate, parsedDate)) {
                Logger::log(LogLevel::Warning, "StockRotationAdvisor", "Data invalida marcada como DADO_INVALIDO para item: " + item.name);
                advice.priority = "DADO_INVÁLIDO";
                advice.recommendation = recommendationForPriority(advice.priority);
                result.items.push_back(advice);
                continue;
            }

            const int remainingDays = daysUntil(parsedDate);
            if (remainingDays < 0) {
                advice.priority = "DESCARTAR";
            } else if (remainingDays <= 30) {
                advice.priority = "CONSUMIR AGORA";
            } else if (remainingDays <= 90) {
                advice.priority = "CONSUMIR PRIMEIRO";
            }
        }

        if (advice.priority.empty()) {
            advice.priority = advice.autonomyMonths < 1.0 ? "MONITORAR" : "ESTÁVEL";
        }

        advice.recommendation = recommendationForPriority(advice.priority);
        result.items.push_back(advice);
    }

    std::sort(result.items.begin(), result.items.end(), [](const StockRotationAdvice &left, const StockRotationAdvice &right) {
        const int leftRank = priorityRank(left.priority);
        const int rightRank = priorityRank(right.priority);
        if (leftRank != rightRank) {
            return leftRank < rightRank;
        }

        if (left.autonomyMonths != right.autonomyMonths) {
            return left.autonomyMonths < right.autonomyMonths;
        }

        if (left.quantity != right.quantity) {
            return left.quantity > right.quantity;
        }

        return left.item < right.item;
    });

    Logger::log(LogLevel::Info, "StockRotationAdvisor", "Recomendacao de rotacao gerada com sucesso.");
    return result;
}

bool StockRotationAdvisor::parseDate(const std::string &value, std::tm &date)
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

int StockRotationAdvisor::daysUntil(const std::tm &date)
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
