#include "CsvInventoryReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "CsvInventoryFormat.h"

CsvResult CsvInventoryReader::ensureFile(const std::filesystem::path &path) const
{
    try {
        if (std::filesystem::exists(path)) {
            return {true, "CSV ja existe."};
        }

        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            return {false, "nao foi possivel criar o arquivo " + path.string()};
        }

        file << kInventoryCsvHeader << '\n';
        return {true, "CSV criado com sucesso."};
    } catch (const std::exception &error) {
        return {false, error.what()};
    }
}

CsvReadResult CsvInventoryReader::read(const std::filesystem::path &path) const
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return {false, "arquivo inexistente ou sem permissao de leitura: " + path.string(), {}};
    }

    std::string line;
    if (!std::getline(file, line)) {
        return {false, "arquivo CSV vazio: " + path.string(), {}};
    }

    if (line != kInventoryCsvHeader) {
        return {false, "cabecalho CSV invalido em " + path.string(), {}};
    }

    CsvReadResult result;
    result.success = true;

    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (line.empty()) {
            continue;
        }

        const std::vector<std::string> fields = parseLine(line);
        if (fields.size() != 8) {
            return {false, "linha " + std::to_string(lineNumber) + " possui quantidade invalida de campos.", {}};
        }

        try {
            InventoryItem item;
            item.name = fields.at(0);
            item.category = fields.at(1);
            item.quantity = parseNumber(fields.at(2));
            item.unit = fields.at(3);
            item.monthlyConsumption = parseNumber(fields.at(4));
            item.unitPrice = parseNumber(fields.at(5));
            item.expirationDate = fields.at(6);
            item.minimumStock = parseNumber(fields.at(7));
            result.items.push_back(item);
        } catch (const std::exception &error) {
            return {false, "linha " + std::to_string(lineNumber) + " contem numero invalido: " + error.what(), {}};
        }
    }

    result.message = "CSV lido com sucesso.";
    return result;
}

std::vector<std::string> CsvInventoryReader::parseLine(const std::string &line)
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

double CsvInventoryReader::parseNumber(const std::string &value)
{
    std::size_t parsedCharacters = 0;
    const double number = std::stod(value, &parsedCharacters);
    if (parsedCharacters != value.size()) {
        throw std::invalid_argument(value);
    }

    return number;
}
