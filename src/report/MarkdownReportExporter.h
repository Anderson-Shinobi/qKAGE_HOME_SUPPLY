#pragma once

#include <filesystem>
#include <string>

struct MarkdownExportResult {
    bool success = false;
    std::string message;
    std::filesystem::path outputPath;
};

class MarkdownReportExporter {
public:
    MarkdownExportResult exportMonthlyReport(
        const std::string &monthlyReport,
        const std::filesystem::path &reportsDirectory) const;

private:
    static std::string currentDate();
    static std::string toMarkdown(const std::string &monthlyReport, const std::string &generationDate);
};
