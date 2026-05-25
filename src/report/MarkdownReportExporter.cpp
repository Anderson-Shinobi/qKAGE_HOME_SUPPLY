#include "MarkdownReportExporter.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../logging/Logger.h"

MarkdownExportResult MarkdownReportExporter::exportMonthlyReport(
    const std::string &monthlyReport,
    const std::filesystem::path &reportsDirectory) const
{
    if (monthlyReport.empty()) {
        Logger::log(LogLevel::Error, "MarkdownReportExporter", "Relatorio mensal vazio para exportacao.");
        return {false, "relatorio mensal vazio.", {}};
    }

    try {
        if (std::filesystem::exists(reportsDirectory) &&
            !std::filesystem::is_directory(reportsDirectory)) {
            Logger::log(LogLevel::Error, "MarkdownReportExporter", "Caminho de relatorios nao e diretorio: " + reportsDirectory.string());
            return {false, "caminho de relatorios nao e um diretorio: " + reportsDirectory.string(), {}};
        }

        std::filesystem::create_directories(reportsDirectory);
    } catch (const std::exception &error) {
        Logger::log(LogLevel::Error, "MarkdownReportExporter", "Falha ao criar diretorio de relatorios: " + std::string(error.what()));
        return {false, "nao foi possivel criar o diretorio de relatorios: " + std::string(error.what()), {}};
    }

    const std::string generationDate = currentDate();
    const std::filesystem::path outputPath = reportsDirectory / ("monthly_report_" + generationDate + ".md");

    std::ofstream file(outputPath);
    if (!file.is_open()) {
        Logger::log(LogLevel::Error, "MarkdownReportExporter", "Nao foi possivel escrever Markdown: " + outputPath.string());
        return {false, "nao foi possivel escrever o arquivo Markdown: " + outputPath.string(), outputPath};
    }

    file << toMarkdown(monthlyReport, generationDate);
    if (!file.good()) {
        Logger::log(LogLevel::Error, "MarkdownReportExporter", "Falha durante escrita Markdown: " + outputPath.string());
        return {false, "falha durante a escrita do arquivo Markdown: " + outputPath.string(), outputPath};
    }

    Logger::log(LogLevel::Info, "MarkdownReportExporter", "Exportacao Markdown concluida: " + outputPath.string());
    return {true, "Exportacao Markdown concluida.", outputPath};
}

std::string MarkdownReportExporter::currentDate()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    const std::tm localTime = *std::localtime(&time);

    std::ostringstream date;
    date << std::put_time(&localTime, "%Y-%m-%d");
    return date.str();
}

std::string MarkdownReportExporter::toMarkdown(const std::string &monthlyReport, const std::string &generationDate)
{
    std::istringstream input(monthlyReport);
    std::ostringstream output;
    std::string line;

    output << "# KAGE Home Supply\n\n";
    output << "Data de geracao: " << generationDate << "\n\n";

    while (std::getline(input, line)) {
        if (line == "KAGE Home Supply - Relatorio mensal consolidado" || line.empty()) {
            continue;
        }

        if (line == "[ESTOQUE]" ||
            line == "[ECONOMIA]" ||
            line == "[COFRINHOS]" ||
            line == "[INVESTIMENTO]" ||
            line == "[RESUMO FINAL]") {
            output << "## " << line.substr(1, line.size() - 2) << "\n\n";
            continue;
        }

        output << "- " << line << '\n';
    }

    return output.str();
}
