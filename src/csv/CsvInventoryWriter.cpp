#include "CsvInventoryWriter.h"

#include <cctype>
#include <fstream>

#include "../logging/Logger.h"
#include "CsvInventoryFormat.h"

namespace {
bool isBlank(const std::string &value)
{
    for (const unsigned char ch : value) {
        if (!std::isspace(ch)) {
            return false;
        }
    }

    return true;
}
}

CsvResult CsvInventoryWriter::addItem(const std::filesystem::path &path, const InventoryItem &item) const
{
    const CsvResult validation = validateItem(item);
    if (!validation.success) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Falha ao validar item para escrita CSV: " + validation.message);
        return validation;
    }

    CsvInventoryReader reader;
    const CsvResult fileResult = reader.ensureFile(path);
    if (!fileResult.success) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Falha ao preparar CSV: " + fileResult.message);
        return fileResult;
    }

    const CsvReadResult readResult = reader.read(path);
    if (!readResult.success) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Falha ao ler CSV antes da escrita: " + readResult.message);
        return {false, readResult.message};
    }

    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Nao foi possivel abrir CSV para escrita: " + path.string());
        return {false, "nao foi possivel abrir o CSV para escrita: " + path.string()};
    }

    file << escapeField(item.name) << ','
         << escapeField(item.category) << ','
         << item.quantity << ','
         << escapeField(item.unit) << ','
         << item.monthlyConsumption << ','
         << item.unitPrice << ','
         << escapeField(item.expirationDate) << ','
         << item.minimumStock << '\n';

    Logger::log(LogLevel::Info, "CsvInventoryWriter", "Item adicionado com sucesso: " + item.name);
    return {true, "Item adicionado ao estoque: " + item.name};
}

CsvResult CsvInventoryWriter::updateItem(const std::filesystem::path &path, const std::string &originalName, const InventoryItem &item) const
{
    const CsvResult validation = validateItem(item);
    if (!validation.success) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Falha ao validar item para edicao CSV: " + validation.message);
        return validation;
    }

    if (isBlank(originalName)) {
        return {false, "item original e obrigatorio."};
    }

    CsvInventoryReader reader;
    const CsvReadResult readResult = reader.read(path);
    if (!readResult.success) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Falha ao ler CSV antes da edicao: " + readResult.message);
        return {false, readResult.message};
    }

    std::vector<InventoryItem> items = readResult.items;
    bool updated = false;
    for (InventoryItem &current : items) {
        if (current.name == originalName) {
            current = item;
            updated = true;
            break;
        }
    }

    if (!updated) {
        return {false, "item nao encontrado para edicao: " + originalName};
    }

    const CsvResult writeResult = writeItems(path, items);
    if (!writeResult.success) {
        return writeResult;
    }

    Logger::log(LogLevel::Info, "CsvInventoryWriter", "Item editado com sucesso: " + originalName);
    return {true, "Item editado no estoque: " + item.name};
}

CsvResult CsvInventoryWriter::removeItem(const std::filesystem::path &path, const std::string &itemName) const
{
    if (isBlank(itemName)) {
        return {false, "item e obrigatorio."};
    }

    CsvInventoryReader reader;
    const CsvReadResult readResult = reader.read(path);
    if (!readResult.success) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Falha ao ler CSV antes da remocao: " + readResult.message);
        return {false, readResult.message};
    }

    std::vector<InventoryItem> items;
    items.reserve(readResult.items.size());
    bool removed = false;
    for (const InventoryItem &current : readResult.items) {
        if (!removed && current.name == itemName) {
            removed = true;
            continue;
        }

        items.push_back(current);
    }

    if (!removed) {
        return {false, "item nao encontrado para remocao: " + itemName};
    }

    const CsvResult writeResult = writeItems(path, items);
    if (!writeResult.success) {
        return writeResult;
    }

    Logger::log(LogLevel::Info, "CsvInventoryWriter", "Item removido com sucesso: " + itemName);
    return {true, "Item removido do estoque: " + itemName};
}

CsvResult CsvInventoryWriter::validateItem(const InventoryItem &item)
{
    if (isBlank(item.name)) {
        return {false, "item e obrigatorio."};
    }

    if (isBlank(item.category)) {
        return {false, "categoria e obrigatoria."};
    }

    if (isBlank(item.unit)) {
        return {false, "unidade e obrigatoria."};
    }

    if (isBlank(item.expirationDate)) {
        return {false, "validade e obrigatoria."};
    }

    if (item.quantity < 0.0) {
        return {false, "quantidade nao pode ser negativa."};
    }

    if (item.monthlyConsumption < 0.0) {
        return {false, "consumo_mensal nao pode ser negativo."};
    }

    if (item.unitPrice < 0.0) {
        return {false, "preco_unitario nao pode ser negativo."};
    }

    if (item.minimumStock < 0.0) {
        return {false, "estoque_minimo nao pode ser negativo."};
    }

    return {true, "Item valido."};
}

CsvResult CsvInventoryWriter::writeItems(const std::filesystem::path &path, const std::vector<InventoryItem> &items)
{
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        Logger::log(LogLevel::Error, "CsvInventoryWriter", "Nao foi possivel abrir CSV para reescrita: " + path.string());
        return {false, "nao foi possivel abrir o CSV para reescrita: " + path.string()};
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

    return {true, "CSV atualizado com sucesso."};
}

std::string CsvInventoryWriter::escapeField(const std::string &value)
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
