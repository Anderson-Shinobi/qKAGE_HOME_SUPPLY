#include "BackupManager.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../logging/Logger.h"

BackupResult BackupManager::createBackup(
    const std::filesystem::path &sourceRoot,
    const std::filesystem::path &backupRoot) const
{
    BackupResult result;

    try {
        if (!std::filesystem::exists(sourceRoot) || !std::filesystem::is_directory(sourceRoot)) {
            Logger::log(LogLevel::Error, "BackupManager", "Diretorio de origem inexistente: " + sourceRoot.string());
            return {false, "diretorio de origem inexistente: " + sourceRoot.string()};
        }

        if (std::filesystem::exists(backupRoot) && !std::filesystem::is_directory(backupRoot)) {
            Logger::log(LogLevel::Error, "BackupManager", "Destino de backups nao e diretorio: " + backupRoot.string());
            return {false, "destino de backups nao e um diretorio: " + backupRoot.string()};
        }

        std::filesystem::create_directories(backupRoot);

        result.backupPath = backupRoot / ("backup_" + timestamp());
        if (std::filesystem::exists(result.backupPath)) {
            Logger::log(LogLevel::Warning, "BackupManager", "Backup ja existente: " + result.backupPath.string());
            return {false, "backup ja existente: " + result.backupPath.string(), result.backupPath};
        }

        const std::filesystem::path dataBackupDir = result.backupPath / "data";
        const std::filesystem::path reportsBackupDir = result.backupPath / "reports";
        std::filesystem::create_directories(dataBackupDir);
        std::filesystem::create_directories(reportsBackupDir);

        const std::filesystem::path dataDir = sourceRoot / "data";
        if (!std::filesystem::exists(dataDir) || std::filesystem::is_empty(dataDir)) {
            Logger::log(LogLevel::Error, "BackupManager", "Diretorio data vazio ou inexistente: " + dataDir.string());
            return {false, "diretorio data vazio ou inexistente: " + dataDir.string(), result.backupPath};
        }

        copyIfExists(dataDir / "estoque.csv", dataBackupDir / "estoque.csv", result);
        copyIfExists(dataDir / "piggybanks.csv", dataBackupDir / "piggybanks.csv", result);

        const std::filesystem::path reportsDir = sourceRoot / "reports";
        if (!std::filesystem::exists(reportsDir)) {
            result.ignoredFiles.push_back("reports/*.md (diretorio inexistente)");
        } else {
            bool copiedReport = false;
            for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(reportsDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".md") {
                    const std::filesystem::path target = reportsBackupDir / entry.path().filename();
                    std::filesystem::copy_file(entry.path(), target, std::filesystem::copy_options::overwrite_existing);
                    result.copiedFiles.push_back(entry.path().string());
                    copiedReport = true;
                }
            }

            if (!copiedReport) {
                result.ignoredFiles.push_back("reports/*.md (nenhum relatorio encontrado)");
            }
        }

        if (result.copiedFiles.empty()) {
            Logger::log(LogLevel::Error, "BackupManager", "Nenhum arquivo critico encontrado para backup.");
            return {false, "nenhum arquivo critico encontrado para backup.", result.backupPath, result.copiedFiles, result.ignoredFiles};
        }

        result.success = true;
        result.message = "Backup concluido.";
        Logger::log(LogLevel::Info, "BackupManager", "Backup concluido em: " + result.backupPath.string());
        return result;
    } catch (const std::exception &error) {
        Logger::log(LogLevel::Error, "BackupManager", "Falha ao criar backup: " + std::string(error.what()));
        return {false, "falha ao criar backup: " + std::string(error.what()), result.backupPath, result.copiedFiles, result.ignoredFiles};
    }
}

std::string BackupManager::timestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    const std::tm localTime = *std::localtime(&time);

    std::ostringstream value;
    value << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S");
    return value.str();
}

bool BackupManager::copyIfExists(
    const std::filesystem::path &source,
    const std::filesystem::path &target,
    BackupResult &result)
{
    if (!std::filesystem::exists(source)) {
        result.ignoredFiles.push_back(source.string() + " (inexistente)");
        return false;
    }

    std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing);
    result.copiedFiles.push_back(source.string());
    return true;
}
