#include "DataIntegrityChecker.h"

#include <fstream>
#include <sstream>

#include "../backup/BackupManager.h"
#include "../csv/CsvInventoryFormat.h"
#include "../history/StockMovementHistory.h"
#include "../logging/Logger.h"

namespace {
constexpr const char *kPiggyBankCsvHeader = "nome,valor_atual,meta,aporte_mensal,status";
}

DataIntegrityReport DataIntegrityChecker::check(const DataIntegrityOptions &options) const
{
    DataIntegrityReport report;
    const std::filesystem::path inventoryPath = options.rootPath / "data" / "estoque.csv";
    const std::filesystem::path piggyBankPath = options.rootPath / "data" / "piggybanks.csv";
    const std::filesystem::path reportsPath = options.rootPath / "reports";
    const std::filesystem::path backupsPath = options.rootPath / "backups";

    report.checkedFiles.push_back(inventoryPath.string());
    report.checkedFiles.push_back(piggyBankPath.string());
    report.checkedFiles.push_back(reportsPath.string());
    report.checkedFiles.push_back(backupsPath.string());

    validateInventoryCsv(inventoryPath, report);
    validatePiggyBankCsv(piggyBankPath, report);

    if (!std::filesystem::exists(reportsPath)) {
        report.warnings.push_back("diretorio reports inexistente: " + reportsPath.string());
    } else if (!std::filesystem::is_directory(reportsPath)) {
        report.errors.push_back("reports nao e um diretorio: " + reportsPath.string());
    }

    if (!std::filesystem::exists(backupsPath)) {
        report.warnings.push_back("diretorio backups inexistente: " + backupsPath.string());
    } else if (!std::filesystem::is_directory(backupsPath)) {
        report.errors.push_back("backups nao e um diretorio: " + backupsPath.string());
    }

    if (options.fix) {
        applyFixes(options.rootPath, inventoryPath, piggyBankPath, report);

        report.errors.clear();
        report.warnings.clear();
        validateInventoryCsv(inventoryPath, report);
        validatePiggyBankCsv(piggyBankPath, report);
        if (!std::filesystem::exists(reportsPath)) {
            report.warnings.push_back("diretorio reports inexistente: " + reportsPath.string());
        }
        if (!std::filesystem::exists(backupsPath)) {
            report.warnings.push_back("diretorio backups inexistente: " + backupsPath.string());
        }
    }

    report.success = report.errors.empty();
    if (!report.errors.empty()) {
        report.status = "FAILED";
        Logger::log(LogLevel::Error, "DataIntegrityChecker", "Verificacao de integridade falhou com erros.");
    } else if (!report.warnings.empty()) {
        report.status = "WARNING";
        Logger::log(LogLevel::Warning, "DataIntegrityChecker", "Verificacao de integridade concluida com avisos.");
    } else {
        report.status = "OK";
        Logger::log(LogLevel::Info, "DataIntegrityChecker", "Verificacao de integridade concluida com sucesso.");
    }

    return report;
}

void DataIntegrityChecker::validateInventoryCsv(const std::filesystem::path &path, DataIntegrityReport &report)
{
    validateCsv(path, kInventoryCsvHeader, 8, {2, 4, 5, 7}, {}, report);
}

void DataIntegrityChecker::validatePiggyBankCsv(const std::filesystem::path &path, DataIntegrityReport &report)
{
    validateCsv(path, kPiggyBankCsvHeader, 5, {1, 3}, {2}, report);
}

void DataIntegrityChecker::validateCsv(
    const std::filesystem::path &path,
    const std::string &expectedHeader,
    int expectedColumns,
    const std::vector<int> &nonNegativeNumericColumns,
    const std::vector<int> &positiveNumericColumns,
    DataIntegrityReport &report)
{
    if (!std::filesystem::exists(path)) {
        report.errors.push_back("CSV inexistente: " + path.string());
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        report.errors.push_back("CSV sem permissao de leitura: " + path.string());
        return;
    }

    std::string line;
    if (!std::getline(file, line)) {
        report.errors.push_back("CSV vazio: " + path.string());
        return;
    }

    if (line != expectedHeader) {
        report.errors.push_back("cabecalho invalido em " + path.string());
    }

    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (isBlank(line)) {
            report.warnings.push_back("linha vazia em " + path.string() + ":" + std::to_string(lineNumber));
            continue;
        }

        const std::vector<std::string> fields = parseLine(line);
        if (static_cast<int>(fields.size()) != expectedColumns) {
            report.errors.push_back("quantidade incorreta de colunas em " + path.string() + ":" + std::to_string(lineNumber));
            continue;
        }

        for (const int column : nonNegativeNumericColumns) {
            if (!isNumber(fields.at(column))) {
                report.errors.push_back("valor numerico invalido em " + path.string() + ":" + std::to_string(lineNumber));
                continue;
            }

            if (std::stod(fields.at(column)) < 0.0) {
                report.errors.push_back("valor negativo invalido em " + path.string() + ":" + std::to_string(lineNumber));
            }
        }

        for (const int column : positiveNumericColumns) {
            if (!isNumber(fields.at(column))) {
                report.errors.push_back("valor numerico invalido em " + path.string() + ":" + std::to_string(lineNumber));
                continue;
            }

            if (std::stod(fields.at(column)) <= 0.0) {
                report.errors.push_back("valor deve ser positivo em " + path.string() + ":" + std::to_string(lineNumber));
            }
        }
    }
}

void DataIntegrityChecker::applyFixes(
    const std::filesystem::path &rootPath,
    const std::filesystem::path &inventoryPath,
    const std::filesystem::path &piggyBankPath,
    DataIntegrityReport &report)
{
    BackupManager backupManager;
    const BackupResult backup = backupManager.createBackup(rootPath, rootPath / "backups");
    if (backup.success) {
        report.fixes.push_back("backup automatico criado: " + backup.backupPath.string());
    } else {
        report.warnings.push_back("backup automatico nao criado: " + backup.message);
    }

    ensureCsvHeader(inventoryPath, kInventoryCsvHeader, report);
    ensureCsvHeader(piggyBankPath, kPiggyBankCsvHeader, report);
    removeBlankLines(inventoryPath, report);
    removeBlankLines(piggyBankPath, report);
}

void DataIntegrityChecker::ensureCsvHeader(const std::filesystem::path &path, const std::string &header, DataIntegrityReport &report)
{
    if (std::filesystem::exists(path)) {
        return;
    }

    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(path);
    file << header << '\n';
    report.fixes.push_back("CSV recriado: " + path.string());

    StockMovementHistory history;
    const StockMovementResult historyResult = history.record(
        path.parent_path() / "stock_movements.csv",
        "REPAIR",
        path.filename().string(),
        0.0,
        "",
        "Arquivo recriado pelo integrity-check --fix");
    if (!historyResult.success) {
        report.warnings.push_back("historico REPAIR nao registrado: " + historyResult.message);
    }
}

void DataIntegrityChecker::removeBlankLines(const std::filesystem::path &path, DataIntegrityReport &report)
{
    if (!std::filesystem::exists(path)) {
        return;
    }

    std::ifstream input(path);
    if (!input.is_open()) {
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool removed = false;
    while (std::getline(input, line)) {
        if (isBlank(line)) {
            removed = true;
            continue;
        }
        lines.push_back(line);
    }
    input.close();

    if (!removed) {
        return;
    }

    std::ofstream output(path, std::ios::trunc);
    for (const std::string &keptLine : lines) {
        output << keptLine << '\n';
    }
    report.fixes.push_back("linhas vazias removidas: " + path.string());

    StockMovementHistory history;
    const StockMovementResult historyResult = history.record(
        path.parent_path() / "stock_movements.csv",
        "ADJUST",
        path.filename().string(),
        0.0,
        "",
        "Linhas vazias removidas pelo integrity-check --fix");
    if (!historyResult.success) {
        report.warnings.push_back("historico ADJUST nao registrado: " + historyResult.message);
    }
}

std::vector<std::string> DataIntegrityChecker::parseLine(const std::string &line)
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

bool DataIntegrityChecker::isBlank(const std::string &value)
{
    for (const char ch : value) {
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
            return false;
        }
    }

    return true;
}

bool DataIntegrityChecker::isNumber(const std::string &value)
{
    try {
        std::size_t parsedCharacters = 0;
        std::stod(value, &parsedCharacters);
        return parsedCharacters == value.size();
    } catch (...) {
        return false;
    }
}
