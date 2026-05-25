#include "StockMovementHistory.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
constexpr const char *kStockMovementHeader = "data_hora,tipo,item,quantidade,unidade,observacao";
}

StockMovementResult StockMovementHistory::ensureFile(const std::filesystem::path &path) const
{
    try {
        if (std::filesystem::exists(path)) {
            return {true, "Historico ja existe."};
        }

        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            return {false, "nao foi possivel criar o historico: " + path.string()};
        }

        file << kStockMovementHeader << '\n';
        return {true, "Historico criado com sucesso."};
    } catch (const std::exception &error) {
        return {false, error.what()};
    }
}

StockMovementResult StockMovementHistory::record(
    const std::filesystem::path &path,
    const std::string &type,
    const std::string &item,
    double quantity,
    const std::string &unit,
    const std::string &observation) const
{
    const StockMovementResult validation = validate(type, item, quantity);
    if (!validation.success) {
        return validation;
    }

    const StockMovementResult fileResult = ensureFile(path);
    if (!fileResult.success) {
        return fileResult;
    }

    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        return {false, "nao foi possivel abrir historico para escrita: " + path.string()};
    }

    file << escapeField(currentDateTime()) << ','
         << escapeField(type) << ','
         << escapeField(item) << ','
         << quantity << ','
         << escapeField(unit) << ','
         << escapeField(observation) << '\n';

    return {true, "Movimentacao registrada: " + type + " - " + item};
}

StockMovementReadResult StockMovementHistory::readAll(const std::filesystem::path &path) const
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return {false, "arquivo de historico inexistente ou sem permissao de leitura: " + path.string(), {}};
    }

    std::string line;
    if (!std::getline(file, line)) {
        return {false, "arquivo de historico vazio: " + path.string(), {}};
    }

    if (line != kStockMovementHeader) {
        return {false, "cabecalho de historico invalido em " + path.string(), {}};
    }

    StockMovementReadResult result;
    result.success = true;

    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (isBlank(line)) {
            continue;
        }

        const std::vector<std::string> fields = parseLine(line);
        if (fields.size() != 6) {
            return {false, "linha " + std::to_string(lineNumber) + " possui quantidade invalida de campos.", {}};
        }

        try {
            StockMovement movement;
            movement.dateTime = fields.at(0);
            movement.type = fields.at(1);
            movement.item = fields.at(2);
            movement.quantity = parseNumber(fields.at(3));
            movement.unit = fields.at(4);
            movement.observation = fields.at(5);
            const StockMovementResult validation = validate(movement.type, movement.item, movement.quantity);
            if (!validation.success) {
                return {false, "linha " + std::to_string(lineNumber) + " invalida: " + validation.message, {}};
            }
            result.movements.push_back(movement);
        } catch (const std::exception &error) {
            return {false, "linha " + std::to_string(lineNumber) + " contem numero invalido: " + error.what(), {}};
        }
    }

    result.message = "Historico lido com sucesso.";
    return result;
}

StockMovementResult StockMovementHistory::validate(const std::string &type, const std::string &item, double quantity)
{
    if (!isAllowedType(type)) {
        return {false, "tipo de movimentacao invalido: " + type};
    }

    if (isBlank(item)) {
        return {false, "item e obrigatorio."};
    }

    if (quantity < 0.0) {
        return {false, "quantidade nao pode ser negativa."};
    }

    return {true, "Movimentacao valida."};
}

bool StockMovementHistory::isAllowedType(const std::string &type)
{
    return type == "ADD" || type == "CONSUME" || type == "ADJUST" || type == "REPAIR";
}

bool StockMovementHistory::isBlank(const std::string &value)
{
    for (const char ch : value) {
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
            return false;
        }
    }

    return true;
}

std::string StockMovementHistory::currentDateTime()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    const std::tm localTime = *std::localtime(&time);

    std::ostringstream value;
    value << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return value.str();
}

std::string StockMovementHistory::escapeField(const std::string &value)
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

std::vector<std::string> StockMovementHistory::parseLine(const std::string &line)
{
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line.at(i);
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line.at(i + 1) == '"') {
                current.push_back('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
            continue;
        }

        if (ch == ',' && !inQuotes) {
            fields.push_back(current);
            current.clear();
            continue;
        }

        current.push_back(ch);
    }

    fields.push_back(current);
    return fields;
}

double StockMovementHistory::parseNumber(const std::string &value)
{
    std::size_t parsedCharacters = 0;
    const double number = std::stod(value, &parsedCharacters);
    if (parsedCharacters != value.size()) {
        throw std::invalid_argument(value);
    }

    return number;
}
