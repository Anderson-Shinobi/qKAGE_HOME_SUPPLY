#include "ReportService.h"

#include <filesystem>
#include <iostream>

#include "../report/MarkdownReportExporter.h"
#include "../report/MonthlyReportGenerator.h"

namespace {
const char *kDefaultCsvPath = "data/estoque.csv";
const char *kDefaultPiggyBankCsvPath = "data/piggybanks.csv";
}

int ReportService::monthlyReport(int argc, char *argv[]) const
{
    const std::filesystem::path inventoryCsvPath = argc >= 3 ? argv[2] : kDefaultCsvPath;
    const std::filesystem::path piggyBankCsvPath = argc == 4 ? argv[3] : kDefaultPiggyBankCsvPath;
    MonthlyReportGenerator generator;
    const MonthlyReportResult result = generator.generate(inventoryCsvPath, piggyBankCsvPath);
    if (!result.success) {
        std::cerr << "Erro ao gerar relatorio mensal: " << result.message << '\n';
        return 1;
    }
    std::cout << result.report;
    return 0;
}

int ReportService::exportReport(int argc, char *argv[]) const
{
    const std::filesystem::path inventoryCsvPath = argc >= 3 ? argv[2] : kDefaultCsvPath;
    const std::filesystem::path piggyBankCsvPath = argc >= 4 ? argv[3] : kDefaultPiggyBankCsvPath;
    const std::filesystem::path reportsDirectory = argc == 5 ? argv[4] : "reports";
    MonthlyReportGenerator generator;
    const MonthlyReportResult monthlyReport = generator.generate(inventoryCsvPath, piggyBankCsvPath);
    if (!monthlyReport.success) {
        std::cerr << "Erro ao exportar relatorio: " << monthlyReport.message << '\n';
        return 1;
    }
    MarkdownReportExporter exporter;
    const MarkdownExportResult exportResult = exporter.exportMonthlyReport(monthlyReport.report, reportsDirectory);
    if (!exportResult.success) {
        std::cerr << "Erro ao exportar relatorio: " << exportResult.message << '\n';
        return 1;
    }
    std::cout << "Status da exportacao: " << exportResult.message << '\n';
    std::cout << "Arquivo gerado: " << exportResult.outputPath.string() << '\n';
    return 0;
}
