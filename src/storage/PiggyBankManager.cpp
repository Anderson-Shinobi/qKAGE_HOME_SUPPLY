#include "PiggyBankManager.h"

#include <cctype>
#include <fstream>
#include <stdexcept>

namespace {
constexpr const char *kPiggyBankCsvHeader = "nome,valor_atual,meta,aporte_mensal,status";

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

PiggyBankResult PiggyBankManager::ensureFile(const std::filesystem::path &path) const
{
    try {
        if (std::filesystem::exists(path)) {
            return {true, "CSV de cofrinhos ja existe."};
        }

        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            return {false, "nao foi possivel criar o arquivo " + path.string()};
        }

        file << kPiggyBankCsvHeader << '\n';
        return {true, "CSV de cofrinhos criado com sucesso."};
    } catch (const std::exception &error) {
        return {false, error.what()};
    }
}

PiggyBankResult PiggyBankManager::addPiggyBank(const std::filesystem::path &path, const PiggyBank &piggyBank) const
{
    const PiggyBankResult validation = validatePiggyBank(piggyBank);
    if (!validation.success) {
        return validation;
    }

    const PiggyBankResult fileResult = ensureFile(path);
    if (!fileResult.success) {
        return fileResult;
    }

    const PiggyBankReadResult readResult = readAll(path);
    if (!readResult.success) {
        return {false, readResult.message};
    }

    const double goalPercentage = (piggyBank.currentValue / piggyBank.goal) * 100.0;
    const std::string status = classifyStatus(goalPercentage);

    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        return {false, "nao foi possivel abrir o CSV de cofrinhos para escrita: " + path.string()};
    }

    file << escapeField(piggyBank.name) << ','
         << piggyBank.currentValue << ','
         << piggyBank.goal << ','
         << piggyBank.monthlyContribution << ','
         << escapeField(status) << '\n';

    return {true, "Cofrinho adicionado: " + piggyBank.name};
}

PiggyBankReadResult PiggyBankManager::readAll(const std::filesystem::path &path) const
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return {false, "arquivo inexistente ou sem permissao de leitura: " + path.string(), {}};
    }

    std::string line;
    if (!std::getline(file, line)) {
        return {false, "arquivo CSV de cofrinhos vazio: " + path.string(), {}};
    }

    if (line != kPiggyBankCsvHeader) {
        return {false, "cabecalho CSV de cofrinhos invalido em " + path.string(), {}};
    }

    PiggyBankReadResult result;
    result.success = true;

    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (line.empty()) {
            continue;
        }

        const std::vector<std::string> fields = parseLine(line);
        if (fields.size() != 5) {
            return {false, "linha " + std::to_string(lineNumber) + " possui quantidade invalida de campos.", {}};
        }

        try {
            PiggyBank piggyBank;
            piggyBank.name = fields.at(0);
            piggyBank.currentValue = parseNumber(fields.at(1));
            piggyBank.goal = parseNumber(fields.at(2));
            piggyBank.monthlyContribution = parseNumber(fields.at(3));
            piggyBank.goalPercentage = piggyBank.goal > 0.0 ? (piggyBank.currentValue / piggyBank.goal) * 100.0 : 0.0;
            piggyBank.status = fields.at(4).empty() ? classifyStatus(piggyBank.goalPercentage) : fields.at(4);
            result.piggyBanks.push_back(piggyBank);
        } catch (const std::exception &error) {
            return {false, "linha " + std::to_string(lineNumber) + " contem numero invalido: " + error.what(), {}};
        }
    }

    result.message = "CSV de cofrinhos lido com sucesso.";
    return result;
}

PiggyBankResult PiggyBankManager::validatePiggyBank(const PiggyBank &piggyBank)
{
    if (isBlank(piggyBank.name)) {
        return {false, "nome e obrigatorio."};
    }

    if (piggyBank.currentValue < 0.0) {
        return {false, "valor_atual deve ser maior ou igual a zero."};
    }

    if (piggyBank.goal <= 0.0) {
        return {false, "meta deve ser maior que zero."};
    }

    if (piggyBank.monthlyContribution < 0.0) {
        return {false, "aporte_mensal deve ser maior ou igual a zero."};
    }

    return {true, "Cofrinho valido."};
}

std::string PiggyBankManager::classifyStatus(double goalPercentage)
{
    if (goalPercentage < 25.0) {
        return "CRÍTICO";
    }

    if (goalPercentage < 75.0) {
        return "ATENÇÃO";
    }

    return "OK";
}

std::string PiggyBankManager::escapeField(const std::string &value)
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

std::vector<std::string> PiggyBankManager::parseLine(const std::string &line)
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

double PiggyBankManager::parseNumber(const std::string &value)
{
    std::size_t parsedCharacters = 0;
    const double number = std::stod(value, &parsedCharacters);
    if (parsedCharacters != value.size()) {
        throw std::invalid_argument(value);
    }

    return number;
}
