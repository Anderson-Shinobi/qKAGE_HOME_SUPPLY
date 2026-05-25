#include "StockService.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../analysis/StockAutonomyAnalyzer.h"
#include "../core/InventoryItem.h"
#include "../csv/CsvInventoryReader.h"
#include "../csv/CsvInventoryWriter.h"
#include "../history/StockMovementHistory.h"
#include "../planning/ExpirationAnalyzer.h"
#include "../planning/ShoppingListGenerator.h"
#include "../planning/StockRotationAdvisor.h"
#include "../storage/StockConsumptionManager.h"

namespace {
const char *kDefaultCsvPath = "data/estoque.csv";
const char *kDefaultHistoryCsvPath = "data/stock_movements.csv";

bool parseDouble(const std::string &value, double &output)
{
    try {
        std::size_t parsedCharacters = 0;
        output = std::stod(value, &parsedCharacters);
        return parsedCharacters == value.size();
    } catch (...) {
        return false;
    }
}

std::filesystem::path historyPathForInventory(const std::filesystem::path &csvPath)
{
    return csvPath.has_parent_path() ? csvPath.parent_path() / "stock_movements.csv" : kDefaultHistoryCsvPath;
}

int readCsv(const std::filesystem::path &csvPath, bool ensureDefaultFile, std::vector<InventoryItem> &items)
{
    CsvInventoryReader reader;
    if (ensureDefaultFile) {
        const CsvResult creationResult = reader.ensureFile(csvPath);
        if (!creationResult.success) {
            std::cerr << "Erro ao criar CSV inicial: " << creationResult.message << '\n';
            return 1;
        }
    }

    const CsvReadResult readResult = reader.read(csvPath);
    if (!readResult.success) {
        std::cerr << "Erro ao ler CSV: " << readResult.message << '\n';
        return 1;
    }

    items = readResult.items;
    return 0;
}

void printInventory(const std::vector<InventoryItem> &items)
{
    if (items.empty()) {
        std::cout << "Estoque vazio. Cadastre itens no CSV para visualizar dados.\n";
        return;
    }

    std::cout << "Estoque atual\n";
    std::cout << "-------------\n";
    for (const InventoryItem &item : items) {
        std::cout << "Item: " << item.name << '\n'
                  << "Categoria: " << item.category << '\n'
                  << "Quantidade: " << item.quantity << ' ' << item.unit << '\n'
                  << "Consumo mensal: " << item.monthlyConsumption << ' ' << item.unit << '\n'
                  << "Preco unitario: R$ " << item.unitPrice << '\n'
                  << "Validade: " << item.expirationDate << '\n'
                  << "Estoque minimo: " << item.minimumStock << ' ' << item.unit << '\n'
                  << "Autonomia: " << item.autonomyInMonths() << " mes(es)\n"
                  << "Status: " << (item.isCritical() ? "CRITICO" : "OK") << "\n\n";
    }
}

void printAutonomyReport(const std::vector<StockAutonomyEntry> &entries)
{
    if (entries.empty()) {
        std::cout << "Estoque vazio. Cadastre itens no CSV para analisar autonomia.\n";
        return;
    }

    std::cout << "Relatorio de autonomia do estoque\n";
    std::cout << "---------------------------------\n";
    std::cout << std::left << std::setw(24) << "item" << std::right
              << std::setw(14) << "quantidade"
              << std::setw(18) << "consumo_mensal"
              << std::setw(19) << "autonomia_meses"
              << "  status\n";

    for (const StockAutonomyEntry &entry : entries) {
        std::cout << std::left << std::setw(24) << entry.item << std::right
                  << std::fixed << std::setprecision(2)
                  << std::setw(14) << entry.quantity
                  << std::setw(18) << entry.monthlyConsumption
                  << std::setw(19) << entry.autonomyMonths
                  << "  " << entry.status << '\n';
    }
}

void printHistoryReport(const std::vector<StockMovement> &movements)
{
    if (movements.empty()) {
        std::cout << "Historico de movimentacoes vazio.\n";
        return;
    }

    std::cout << "Relatorio de movimentacoes do estoque\n";
    std::cout << "-------------------------------------\n";
    std::cout << std::left << std::setw(22) << "data/hora"
              << std::setw(10) << "tipo"
              << std::setw(24) << "item"
              << std::right << std::setw(12) << "quantidade"
              << "  " << std::left << std::setw(10) << "unidade"
              << "observacao\n";

    for (const StockMovement &movement : movements) {
        std::cout << std::left << std::setw(22) << movement.dateTime
                  << std::setw(10) << movement.type
                  << std::setw(24) << movement.item
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << movement.quantity
                  << "  " << std::left << std::setw(10) << movement.unit
                  << movement.observation << '\n';
    }
}

void printShoppingList(const std::vector<ShoppingListItem> &items)
{
    if (items.empty()) {
        std::cout << "Nenhum item precisa de compra no momento.\n";
        return;
    }

    std::cout << "Lista automatica de compras\n";
    std::cout << "---------------------------\n";
    std::cout << std::left << std::setw(22) << "item"
              << std::setw(16) << "categoria"
              << std::right << std::setw(18) << "quantidade atual"
              << std::setw(17) << "estoque minimo"
              << std::setw(17) << "consumo mensal"
              << std::setw(18) << "autonomia"
              << std::setw(20) << "sugestao compra"
              << std::setw(18) << "custo estimado" << '\n';

    for (const ShoppingListItem &item : items) {
        std::cout << std::left << std::setw(22) << item.item
                  << std::setw(16) << item.category
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(18) << item.currentQuantity
                  << std::setw(17) << item.minimumStock
                  << std::setw(17) << item.monthlyConsumption
                  << std::setw(18) << item.estimatedAutonomy
                  << std::setw(20) << item.suggestedPurchase
                  << ' ' << std::left << std::setw(10) << item.unit
                  << std::right << "R$ " << std::setw(12) << item.estimatedCost << '\n';
    }
}

void printExpirationReport(const std::vector<ExpirationItem> &items)
{
    if (items.empty()) {
        std::cout << "Nenhum item cadastrado para analise de validade.\n";
        return;
    }

    std::cout << "Relatorio de validade e vencimento\n";
    std::cout << "----------------------------------\n";
    std::cout << std::left << std::setw(24) << "item"
              << std::setw(16) << "categoria"
              << std::right << std::setw(12) << "quantidade"
              << "  " << std::left << std::setw(14) << "validade"
              << std::right << std::setw(16) << "dias restantes"
              << "  status\n";

    for (const ExpirationItem &item : items) {
        const std::string daysText = item.status == "SEM_VALIDADE" ? "N/A" : std::to_string(item.remainingDays);
        std::cout << std::left << std::setw(24) << item.item
                  << std::setw(16) << item.category
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << item.quantity
                  << "  " << std::left << std::setw(14) << item.expirationDate
                  << std::right << std::setw(16) << daysText
                  << "  " << item.status << '\n';
    }
}

void printRotationAdvice(const std::vector<StockRotationAdvice> &items)
{
    if (items.empty()) {
        std::cout << "Nenhum item cadastrado para recomendacao de rotacao.\n";
        return;
    }

    std::cout << "Relatorio de rotacao inteligente de estoque\n";
    std::cout << "-------------------------------------------\n";
    std::cout << std::left << std::setw(22) << "item"
              << std::setw(16) << "categoria"
              << std::right << std::setw(12) << "quantidade"
              << "  " << std::left << std::setw(14) << "validade"
              << std::right << std::setw(18) << "autonomia_meses"
              << "  " << std::left << std::setw(18) << "prioridade"
              << "recomendacao\n";

    for (const StockRotationAdvice &item : items) {
        std::cout << std::left << std::setw(22) << item.item
                  << std::setw(16) << item.category
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << item.quantity
                  << "  " << std::left << std::setw(14) << item.expirationDate
                  << std::right << std::setw(18) << item.autonomyMonths
                  << "  " << std::left << std::setw(18) << item.priority
                  << item.recommendation << '\n';
    }
}
}

int StockService::list(int argc, char *argv[]) const
{
    const bool usingDefaultPath = argc == 1 || argc == 2;
    const std::filesystem::path csvPath = argc == 3 ? argv[2] : kDefaultCsvPath;
    std::vector<InventoryItem> items;
    const int readStatus = readCsv(csvPath, usingDefaultPath, items);
    if (readStatus != 0) return readStatus;
    std::cout << "KAGE Home Supply - Sistema CSV inicial\n";
    std::cout << "Arquivo: " << csvPath.string() << "\n\n";
    printInventory(items);
    return 0;
}

int StockService::add(int argc, char *argv[]) const
{
    InventoryItem item;
    item.name = argv[2];
    item.category = argv[3];
    item.unit = argv[5];
    item.expirationDate = argv[8];
    if (!parseDouble(argv[4], item.quantity) ||
        !parseDouble(argv[6], item.monthlyConsumption) ||
        !parseDouble(argv[7], item.unitPrice) ||
        !parseDouble(argv[9], item.minimumStock)) {
        std::cerr << "Erro ao adicionar item: campos numericos invalidos.\n";
        return 1;
    }

    const std::filesystem::path csvPath = argc == 11 ? argv[10] : kDefaultCsvPath;
    CsvInventoryWriter writer;
    const CsvResult result = writer.addItem(csvPath, item);
    if (!result.success) {
        std::cerr << "Erro ao adicionar item: " << result.message << '\n';
        return 1;
    }

    StockMovementHistory history;
    const StockMovementResult historyResult = history.record(
        historyPathForInventory(csvPath), "ADD", item.name, item.quantity, item.unit, "Item adicionado ao estoque");
    if (!historyResult.success) {
        std::cerr << "Aviso: nao foi possivel registrar historico: " << historyResult.message << '\n';
    }

    std::cout << result.message << '\n';
    std::cout << "Arquivo atualizado: " << csvPath.string() << '\n';
    return 0;
}

int StockService::edit(int argc, char *argv[]) const
{
    const std::string originalName = argv[2];

    InventoryItem item;
    item.name = argv[3];
    item.category = argv[4];
    item.unit = argv[6];
    item.expirationDate = argv[9];
    if (!parseDouble(argv[5], item.quantity) ||
        !parseDouble(argv[7], item.monthlyConsumption) ||
        !parseDouble(argv[8], item.unitPrice) ||
        !parseDouble(argv[10], item.minimumStock)) {
        std::cerr << "Erro ao editar item: campos numericos invalidos.\n";
        return 1;
    }

    const std::filesystem::path csvPath = argc == 12 ? argv[11] : kDefaultCsvPath;
    CsvInventoryWriter writer;
    const CsvResult result = writer.updateItem(csvPath, originalName, item);
    if (!result.success) {
        std::cerr << "Erro ao editar item: " << result.message << '\n';
        return 1;
    }

    StockMovementHistory history;
    const StockMovementResult historyResult = history.record(
        historyPathForInventory(csvPath), "ADJUST", item.name, item.quantity, item.unit, "Item editado no estoque");
    if (!historyResult.success) {
        std::cerr << "Aviso: nao foi possivel registrar historico: " << historyResult.message << '\n';
    }

    std::cout << result.message << '\n';
    std::cout << "Arquivo atualizado: " << csvPath.string() << '\n';
    return 0;
}

int StockService::remove(int argc, char *argv[]) const
{
    const std::string itemName = argv[2];
    const std::filesystem::path csvPath = argc == 4 ? argv[3] : kDefaultCsvPath;

    if (itemName.empty()) {
        std::cerr << "Erro ao remover item: item e obrigatorio.\n";
        return 1;
    }

    std::vector<InventoryItem> items;
    const int readStatus = readCsv(csvPath, false, items);
    if (readStatus != 0) {
        return readStatus;
    }

    InventoryItem removedItem;
    bool found = false;
    for (const InventoryItem &item : items) {
        if (item.name == itemName) {
            removedItem = item;
            found = true;
            break;
        }
    }

    if (!found) {
        std::cerr << "Erro ao remover item: item nao encontrado para remocao: " << itemName << '\n';
        return 1;
    }

    CsvInventoryWriter writer;
    const CsvResult result = writer.removeItem(csvPath, itemName);
    if (!result.success) {
        std::cerr << "Erro ao remover item: " << result.message << '\n';
        return 1;
    }

    StockMovementHistory history;
    const StockMovementResult historyResult = history.record(
        historyPathForInventory(csvPath), "ADJUST", removedItem.name, removedItem.quantity, removedItem.unit, "Item removido do estoque");
    if (!historyResult.success) {
        std::cerr << "Aviso: nao foi possivel registrar historico: " << historyResult.message << '\n';
    }

    std::cout << result.message << '\n';
    std::cout << "Arquivo atualizado: " << csvPath.string() << '\n';
    return 0;
}

int StockService::consume(int argc, char *argv[]) const
{
    double quantity = 0.0;
    if (!parseDouble(argv[3], quantity)) {
        std::cerr << "Erro ao consumir item: quantidade invalida.\n";
        return 1;
    }

    const std::filesystem::path csvPath = argc == 6 ? argv[5] : kDefaultCsvPath;
    StockConsumptionManager manager;
    const StockConsumptionResult result = manager.consume(csvPath, historyPathForInventory(csvPath), argv[2], quantity, argv[4]);
    if (!result.success) {
        std::cerr << "Erro ao consumir item: " << result.message << '\n';
        return 1;
    }

    std::cout << "Relatorio de consumo\n";
    std::cout << "--------------------\n";
    std::cout << std::fixed << std::setprecision(2)
              << "Item consumido: " << result.item << '\n'
              << "Quantidade anterior: " << result.previousQuantity << ' ' << result.unit << '\n'
              << "Quantidade consumida: " << result.consumedQuantity << ' ' << result.unit << '\n'
              << "Quantidade atual: " << result.currentQuantity << ' ' << result.unit << '\n'
              << "Status final: " << result.message << '\n';
    std::cout << "Arquivo atualizado: " << csvPath.string() << '\n';
    return 0;
}

int StockService::autonomy(int argc, char *argv[]) const
{
    const bool usingDefaultPath = argc == 2;
    const std::filesystem::path csvPath = argc == 3 ? argv[2] : kDefaultCsvPath;
    std::vector<InventoryItem> items;
    const int readStatus = readCsv(csvPath, usingDefaultPath, items);
    if (readStatus != 0) return readStatus;
    const StockAutonomyAnalysis analysis = analyzeStockAutonomy(items);
    if (!analysis.success) {
        std::cerr << "Erro na analise de autonomia: " << analysis.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Analise de autonomia\n";
    std::cout << "Arquivo: " << csvPath.string() << "\n\n";
    printAutonomyReport(analysis.entries);
    return 0;
}

int StockService::shoppingList(int argc, char *argv[]) const
{
    const std::filesystem::path csvPath = argc == 3 ? argv[2] : kDefaultCsvPath;
    ShoppingListGenerator generator;
    const ShoppingListResult result = generator.generate(csvPath);
    if (!result.success) {
        std::cerr << "Erro ao gerar lista de compras: " << result.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Lista de compras\n";
    std::cout << "Arquivo: " << csvPath.string() << "\n\n";
    printShoppingList(result.items);
    return 0;
}

int StockService::expirationReport(int argc, char *argv[]) const
{
    const std::filesystem::path csvPath = argc == 3 ? argv[2] : kDefaultCsvPath;
    ExpirationAnalyzer analyzer;
    const ExpirationAnalysisResult result = analyzer.analyze(csvPath);
    if (!result.success) {
        std::cerr << "Erro ao gerar relatorio de validade: " << result.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Validade e vencimento\n";
    std::cout << "Arquivo: " << csvPath.string() << "\n\n";
    printExpirationReport(result.items);
    return 0;
}

int StockService::rotationAdvice(int argc, char *argv[]) const
{
    const std::filesystem::path csvPath = argc == 3 ? argv[2] : kDefaultCsvPath;
    StockRotationAdvisor advisor;
    const StockRotationResult result = advisor.advise(csvPath);
    if (!result.success) {
        std::cerr << "Erro ao gerar rotacao inteligente: " << result.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Rotacao inteligente de estoque\n";
    std::cout << "Arquivo: " << csvPath.string() << "\n\n";
    printRotationAdvice(result.items);
    return 0;
}

int StockService::historyReport(int argc, char *argv[]) const
{
    const std::filesystem::path historyPath = argc == 3 ? argv[2] : kDefaultHistoryCsvPath;
    StockMovementHistory history;
    const StockMovementResult fileResult = history.ensureFile(historyPath);
    if (!fileResult.success) {
        std::cerr << "Erro ao preparar historico: " << fileResult.message << '\n';
        return 1;
    }
    const StockMovementReadResult readResult = history.readAll(historyPath);
    if (!readResult.success) {
        std::cerr << "Erro ao ler historico: " << readResult.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Historico de estoque\n";
    std::cout << "Arquivo: " << historyPath.string() << "\n\n";
    printHistoryReport(readResult.movements);
    return 0;
}
