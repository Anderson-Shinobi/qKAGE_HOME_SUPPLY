#include "FinanceService.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

#include "../analysis/CompoundInterestProjection.h"
#include "../analysis/InvestmentCapitalAnalyzer.h"
#include "../analysis/PurchaseSavingsAnalyzer.h"
#include "../storage/PiggyBankManager.h"

namespace {
const char *kDefaultPiggyBankCsvPath = "data/piggybanks.csv";

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

void printSavingsReport(const PurchaseSavingsAnalysis &analysis)
{
    std::cout << "Relatorio de economia por compra\n";
    std::cout << "--------------------------------\n";
    std::cout << std::fixed << std::setprecision(2)
              << "Item: " << analysis.item << '\n'
              << "Quantidade: " << analysis.quantity << '\n'
              << "Preco local: R$ " << analysis.localUnitPrice << '\n'
              << "Preco atacado: R$ " << analysis.wholesaleUnitPrice << '\n'
              << "Economia unitaria: R$ " << analysis.unitSavings << '\n'
              << "Economia total: R$ " << analysis.totalSavings << '\n'
              << "Status: " << analysis.status << '\n';
}

void printInvestmentReport(const InvestmentCapitalAnalysis &analysis)
{
    std::cout << "Relatorio de capital liberado para investimento\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << std::fixed << std::setprecision(2)
              << "Economia mensal: R$ " << analysis.monthlySavings << '\n'
              << "Periodo em meses: " << analysis.months << '\n'
              << "Aporte inicial: R$ " << analysis.initialContribution << '\n'
              << "Capital total liberado: R$ " << analysis.totalReleasedCapital << '\n';
}

void printCompoundInterestReport(const CompoundInterestProjection &projection)
{
    std::cout << "Relatorio de projecao com juros compostos\n";
    std::cout << "-----------------------------------------\n";
    std::cout << std::fixed << std::setprecision(2)
              << "Aporte mensal: R$ " << projection.monthlyContribution << '\n'
              << "Taxa mensal: " << std::setprecision(4) << projection.monthlyRate << '\n'
              << std::setprecision(2)
              << "Periodo em meses: " << projection.months << '\n'
              << "Aporte inicial: R$ " << projection.initialContribution << '\n'
              << "Total aportado: R$ " << projection.totalContributed << '\n'
              << "Juros acumulados: R$ " << projection.accumulatedInterest << '\n'
              << "Valor final estimado: R$ " << projection.finalEstimatedValue << '\n';
}

void printPiggyBankReport(const std::vector<PiggyBank> &piggyBanks)
{
    if (piggyBanks.empty()) {
        std::cout << "Nenhum cofrinho cadastrado.\n";
        return;
    }

    std::cout << "Relatorio de cofrinhos estrategicos\n";
    std::cout << "-----------------------------------\n";
    std::cout << std::left << std::setw(24) << "nome"
              << std::right << std::setw(16) << "valor atual"
              << std::setw(12) << "meta"
              << std::setw(17) << "aporte mensal"
              << std::setw(19) << "percentual meta"
              << "  status\n";

    for (const PiggyBank &piggyBank : piggyBanks) {
        std::cout << std::left << std::setw(24) << piggyBank.name
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(16) << piggyBank.currentValue
                  << std::setw(12) << piggyBank.goal
                  << std::setw(17) << piggyBank.monthlyContribution
                  << std::setw(18) << piggyBank.goalPercentage << "%"
                  << "  " << piggyBank.status << '\n';
    }
}
}

int FinanceService::savings(int, char *argv[]) const
{
    PurchaseSavingsInput input;
    input.item = argv[2];
    if (!parseDouble(argv[3], input.quantity) ||
        !parseDouble(argv[4], input.localUnitPrice) ||
        !parseDouble(argv[5], input.wholesaleUnitPrice)) {
        std::cerr << "Erro na analise de economia: campos numericos invalidos.\n";
        return 1;
    }
    const PurchaseSavingsAnalysis analysis = analyzePurchaseSavings(input);
    if (!analysis.success) {
        std::cerr << "Erro na analise de economia: " << analysis.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Economia por compra\n\n";
    printSavingsReport(analysis);
    return 0;
}

int FinanceService::invest(int argc, char *argv[]) const
{
    InvestmentCapitalInput input;
    if (!parseDouble(argv[2], input.monthlySavings) || !parseDouble(argv[3], input.months)) {
        std::cerr << "Erro na analise de investimento: campos numericos invalidos.\n";
        return 1;
    }
    if (argc == 5 && !parseDouble(argv[4], input.initialContribution)) {
        std::cerr << "Erro na analise de investimento: campos numericos invalidos.\n";
        return 1;
    }
    const InvestmentCapitalAnalysis analysis = analyzeInvestmentCapital(input);
    if (!analysis.success) {
        std::cerr << "Erro na analise de investimento: " << analysis.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Capital liberado para investimento\n\n";
    printInvestmentReport(analysis);
    return 0;
}

int FinanceService::compound(int argc, char *argv[]) const
{
    CompoundInterestInput input;
    if (!parseDouble(argv[2], input.monthlyContribution) ||
        !parseDouble(argv[3], input.monthlyRate) ||
        !parseDouble(argv[4], input.months)) {
        std::cerr << "Erro na projecao de juros compostos: campos numericos invalidos.\n";
        return 1;
    }
    if (argc == 6 && !parseDouble(argv[5], input.initialContribution)) {
        std::cerr << "Erro na projecao de juros compostos: campos numericos invalidos.\n";
        return 1;
    }
    const CompoundInterestProjection projection = projectCompoundInterest(input);
    if (!projection.success) {
        std::cerr << "Erro na projecao de juros compostos: " << projection.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Projecao de investimento\n\n";
    printCompoundInterestReport(projection);
    return 0;
}

int FinanceService::piggyAdd(int argc, char *argv[]) const
{
    PiggyBank piggyBank;
    piggyBank.name = argv[2];
    if (!parseDouble(argv[3], piggyBank.currentValue) ||
        !parseDouble(argv[4], piggyBank.goal) ||
        !parseDouble(argv[5], piggyBank.monthlyContribution)) {
        std::cerr << "Erro ao adicionar cofrinho: campos numericos invalidos.\n";
        return 1;
    }
    const std::filesystem::path csvPath = argc == 7 ? argv[6] : kDefaultPiggyBankCsvPath;
    PiggyBankManager manager;
    const PiggyBankResult result = manager.addPiggyBank(csvPath, piggyBank);
    if (!result.success) {
        std::cerr << "Erro ao adicionar cofrinho: " << result.message << '\n';
        return 1;
    }
    std::cout << result.message << '\n';
    std::cout << "Arquivo atualizado: " << csvPath.string() << '\n';
    return 0;
}

int FinanceService::piggyReport(int argc, char *argv[]) const
{
    const std::filesystem::path csvPath = argc == 3 ? argv[2] : kDefaultPiggyBankCsvPath;
    PiggyBankManager manager;
    const PiggyBankResult fileResult = manager.ensureFile(csvPath);
    if (!fileResult.success) {
        std::cerr << "Erro ao preparar CSV de cofrinhos: " << fileResult.message << '\n';
        return 1;
    }
    const PiggyBankReadResult readResult = manager.readAll(csvPath);
    if (!readResult.success) {
        std::cerr << "Erro ao ler cofrinhos: " << readResult.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Cofrinhos estrategicos\n";
    std::cout << "Arquivo: " << csvPath.string() << "\n\n";
    printPiggyBankReport(readResult.piggyBanks);
    return 0;
}
