#include "MonthlyReportGenerator.h"

#include <iomanip>
#include <sstream>

#include "../analysis/CompoundInterestProjection.h"
#include "../analysis/InvestmentCapitalAnalyzer.h"
#include "../analysis/StockAutonomyAnalyzer.h"
#include "../csv/CsvInventoryReader.h"
#include "../logging/Logger.h"
#include "../storage/PiggyBankManager.h"

namespace {
struct StockStatusCount {
    int critical = 0;
    int attention = 0;
    int ok = 0;
};

StockStatusCount countStockStatuses(const std::vector<StockAutonomyEntry> &entries)
{
    StockStatusCount count;

    for (const StockAutonomyEntry &entry : entries) {
        if (entry.status == "CRÍTICO") {
            ++count.critical;
        } else if (entry.status == "ATENÇÃO") {
            ++count.attention;
        } else if (entry.status == "OK") {
            ++count.ok;
        }
    }

    return count;
}

MonthlyReportResult validatePiggyBanks(const std::vector<PiggyBank> &piggyBanks)
{
    for (const PiggyBank &piggyBank : piggyBanks) {
        if (piggyBank.name.empty()) {
            return {false, "cofrinho com nome vazio."};
        }

        if (piggyBank.currentValue < 0.0) {
            return {false, "cofrinho com valor_atual invalido: " + piggyBank.name};
        }

        if (piggyBank.goal <= 0.0) {
            return {false, "cofrinho com meta invalida: " + piggyBank.name};
        }

        if (piggyBank.monthlyContribution < 0.0) {
            return {false, "cofrinho com aporte_mensal invalido: " + piggyBank.name};
        }
    }

    return {true, "Cofrinhos validos."};
}

double averageGoalPercentage(const std::vector<PiggyBank> &piggyBanks)
{
    if (piggyBanks.empty()) {
        return 0.0;
    }

    double total = 0.0;
    for (const PiggyBank &piggyBank : piggyBanks) {
        total += piggyBank.goalPercentage;
    }

    return total / static_cast<double>(piggyBanks.size());
}
}

MonthlyReportResult MonthlyReportGenerator::generate(
    const std::filesystem::path &inventoryCsvPath,
    const std::filesystem::path &piggyBankCsvPath) const
{
    CsvInventoryReader inventoryReader;
    const CsvReadResult inventoryResult = inventoryReader.read(inventoryCsvPath);
    if (!inventoryResult.success) {
        Logger::log(LogLevel::Error, "MonthlyReportGenerator", "Falha ao ler estoque: " + inventoryResult.message);
        return {false, "falha ao ler estoque: " + inventoryResult.message};
    }

    const StockAutonomyAnalysis stockAnalysis = analyzeStockAutonomy(inventoryResult.items);
    if (!stockAnalysis.success) {
        Logger::log(LogLevel::Error, "MonthlyReportGenerator", "Dados invalidos no estoque: " + stockAnalysis.message);
        return {false, "dados invalidos no estoque: " + stockAnalysis.message};
    }

    PiggyBankManager piggyBankManager;
    const PiggyBankResult piggyFileResult = piggyBankManager.ensureFile(piggyBankCsvPath);
    if (!piggyFileResult.success) {
        Logger::log(LogLevel::Error, "MonthlyReportGenerator", "Falha ao preparar cofrinhos: " + piggyFileResult.message);
        return {false, "falha ao preparar cofrinhos: " + piggyFileResult.message};
    }

    const PiggyBankReadResult piggyResult = piggyBankManager.readAll(piggyBankCsvPath);
    if (!piggyResult.success) {
        Logger::log(LogLevel::Error, "MonthlyReportGenerator", "Falha ao ler cofrinhos: " + piggyResult.message);
        return {false, "falha ao ler cofrinhos: " + piggyResult.message};
    }

    const MonthlyReportResult piggyValidation = validatePiggyBanks(piggyResult.piggyBanks);
    if (!piggyValidation.success) {
        Logger::log(LogLevel::Error, "MonthlyReportGenerator", "Validacao de cofrinhos falhou: " + piggyValidation.message);
        return piggyValidation;
    }

    const StockStatusCount statusCount = countStockStatuses(stockAnalysis.entries);
    const double estimatedSavingsTotal = 0.0;

    const InvestmentCapitalAnalysis capitalAnalysis = analyzeInvestmentCapital({
        estimatedSavingsTotal,
        1.0,
        0.0
    });
    if (!capitalAnalysis.success) {
        Logger::log(LogLevel::Error, "MonthlyReportGenerator", "Falha ao calcular capital liberado: " + capitalAnalysis.message);
        return {false, "falha ao calcular capital liberado: " + capitalAnalysis.message};
    }

    const CompoundInterestProjection investmentProjection = projectCompoundInterest({
        capitalAnalysis.totalReleasedCapital,
        0.0,
        1.0,
        0.0
    });
    if (!investmentProjection.success) {
        Logger::log(LogLevel::Error, "MonthlyReportGenerator", "Falha ao projetar investimento: " + investmentProjection.message);
        return {false, "falha ao projetar investimento: " + investmentProjection.message};
    }

    const double averagePiggyPercentage = averageGoalPercentage(piggyResult.piggyBanks);

    std::ostringstream report;
    report << std::fixed << std::setprecision(2);
    report << "KAGE Home Supply - Relatorio mensal consolidado\n\n";
    report << "[ESTOQUE]\n";
    report << "Total de itens cadastrados: " << inventoryResult.items.size() << '\n';
    report << "Itens CRÍTICO: " << statusCount.critical << '\n';
    report << "Itens ATENÇÃO: " << statusCount.attention << '\n';
    report << "Itens OK: " << statusCount.ok << "\n\n";

    report << "[ECONOMIA]\n";
    report << "Economia total estimada: R$ " << estimatedSavingsTotal << '\n';
    report << "Observacao: sem historico persistido de compras, a economia consolidada permanece zerada.\n\n";

    report << "[COFRINHOS]\n";
    report << "Cofrinhos cadastrados: " << piggyResult.piggyBanks.size() << '\n';
    report << "Percentual medio das metas: " << averagePiggyPercentage << "%\n";
    if (piggyResult.piggyBanks.empty()) {
        report << "Status: nenhum cofrinho cadastrado.\n";
    }
    report << '\n';

    report << "[INVESTIMENTO]\n";
    report << "Capital liberado para investimento: R$ " << capitalAnalysis.totalReleasedCapital << '\n';
    report << "Valor final estimado: R$ " << investmentProjection.finalEstimatedValue << "\n\n";

    report << "[RESUMO FINAL]\n";
    report << "Estoque monitorado: " << (inventoryResult.items.empty() ? "sem itens cadastrados" : "ativo") << '\n';
    report << "Economia mensal consolidada: R$ " << estimatedSavingsTotal << '\n';
    report << "Cofrinhos monitorados: " << piggyResult.piggyBanks.size() << '\n';

    Logger::log(LogLevel::Info, "MonthlyReportGenerator", "Relatorio mensal consolidado gerado.");
    return {true, "Relatorio mensal consolidado gerado.", report.str()};
}
