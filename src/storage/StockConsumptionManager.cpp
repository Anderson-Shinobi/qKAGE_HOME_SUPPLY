#include "StockConsumptionManager.h"

#include <fstream>

#include "../core/InventoryItem.h"
#include "../csv/CsvInventoryFormat.h"
#include "../csv/CsvInventoryReader.h"
#include "../history/StockMovementHistory.h"
#include "../logging/Logger.h"

StockConsumptionResult StockConsumptionManager::consume(
    const std::filesystem::path &inventoryCsvPath,
    const std::filesystem::path &historyCsvPath,
    const std::string &itemName,
    double quantity,
    const std::string &observation) const
{
    if (isBlank(itemName)) {
        Logger::log(LogLevel::Error, "StockConsumptionManager", "Tentativa de consumo com item vazio.");
        return {false, "item e obrigatorio."};
    }

    if (quantity <= 0.0) {
        Logger::log(LogLevel::Error, "StockConsumptionManager", "Tentativa de consumo com quantidade invalida para item: " + itemName);
        return {false, "quantidade deve ser maior que zero."};
    }

    CsvInventoryReader reader;
    const CsvReadResult readResult = reader.read(inventoryCsvPath);
    if (!readResult.success) {
        Logger::log(LogLevel::Error, "StockConsumptionManager", "Falha ao ler estoque para consumo: " + readResult.message);
        return {false, readResult.message};
    }

    std::vector<InventoryItem> items = readResult.items;
    for (InventoryItem &item : items) {
        if (item.name != itemName) {
            continue;
        }

        if (item.quantity < quantity) {
            Logger::log(LogLevel::Warning, "StockConsumptionManager", "Estoque insuficiente para consumo do item: " + itemName);
            return {false, "estoque insuficiente para o item: " + itemName};
        }

        const double previousQuantity = item.quantity;
        item.quantity -= quantity;

        if (writeInventoryCsv(inventoryCsvPath, items) != 0) {
            Logger::log(LogLevel::Error, "StockConsumptionManager", "Falha ao atualizar estoque apos consumo: " + inventoryCsvPath.string());
            return {false, "nao foi possivel atualizar estoque: " + inventoryCsvPath.string()};
        }

        StockMovementHistory history;
        const StockMovementResult historyResult = history.record(
            historyCsvPath,
            "CONSUME",
            item.name,
            quantity,
            item.unit,
            observation.empty() ? "Item consumido do estoque" : observation);
        if (!historyResult.success) {
            Logger::log(LogLevel::Error, "StockConsumptionManager", "Historico falhou apos consumo: " + historyResult.message);
            return {false, "estoque atualizado, mas historico falhou: " + historyResult.message};
        }

        Logger::log(LogLevel::Info, "StockConsumptionManager", "Consumo registrado com sucesso para item: " + item.name);
        return {
            true,
            "Consumo registrado com sucesso.",
            item.name,
            previousQuantity,
            quantity,
            item.quantity,
            item.unit
        };
    }

    Logger::log(LogLevel::Warning, "StockConsumptionManager", "Item nao encontrado para consumo: " + itemName);
    return {false, "item nao encontrado: " + itemName};
}

int StockConsumptionManager::writeInventoryCsv(const std::filesystem::path &path, const std::vector<InventoryItem> &items)
{
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        return 1;
    }

    file << kInventoryCsvHeader << '\n';
    for (const InventoryItem &item : items) {
        file << escapeField(item.name) << ','
             << escapeField(item.category) << ','
             << item.quantity << ','
             << escapeField(item.unit) << ','
             << item.monthlyConsumption << ','
             << item.unitPrice << ','
             << escapeField(item.expirationDate) << ','
             << item.minimumStock << '\n';
    }

    return file.good() ? 0 : 1;
}

std::string StockConsumptionManager::escapeField(const std::string &value)
{
    bool mustQuote = false;
    std::string escaped;
    escaped.reserve(value.size());

    for (const char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
            mustQuote = true;
            continue;
        }

        if (ch == ',' || ch == '\n' || ch == '\r') {
            mustQuote = true;
        }

        escaped.push_back(ch);
    }

    if (!mustQuote) {
        return escaped;
    }

    return '"' + escaped + '"';
}

bool StockConsumptionManager::isBlank(const std::string &value)
{
    for (const char ch : value) {
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
            return false;
        }
    }

    return true;
}
