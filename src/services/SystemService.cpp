#include "SystemService.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

#include "../backup/BackupManager.h"
#include "../config/ConfigManager.h"
#include "../core/ErrorHandler.h"
#include "../logging/Logger.h"
#include "../security/DataIntegrityChecker.h"

namespace {
const char *kDefaultConfigPath = "data/config.ini";

void printBackupReport(const BackupResult &result)
{
    std::cout << "Relatorio de backup\n";
    std::cout << "-------------------\n";
    std::cout << "Backup gerado em: " << result.backupPath.string() << '\n';
    std::cout << "Arquivos copiados: " << result.copiedFiles.size() << '\n';
    for (const std::string &file : result.copiedFiles) std::cout << "  COPIADO: " << file << '\n';
    std::cout << "Arquivos ignorados: " << result.ignoredFiles.size() << '\n';
    for (const std::string &file : result.ignoredFiles) std::cout << "  IGNORADO: " << file << '\n';
    std::cout << "Status final: " << result.message << '\n';
}

void printIntegrityReport(const DataIntegrityReport &report)
{
    std::cout << "Relatorio de integridade dos dados\n";
    std::cout << "----------------------------------\n";
    std::cout << "Arquivos verificados: " << report.checkedFiles.size() << '\n';
    for (const std::string &file : report.checkedFiles) std::cout << "  VERIFICADO: " << file << '\n';
    std::cout << "Erros encontrados: " << report.errors.size() << '\n';
    for (const std::string &error : report.errors) std::cout << "  ERRO: " << error << '\n';
    std::cout << "Avisos encontrados: " << report.warnings.size() << '\n';
    for (const std::string &warning : report.warnings) std::cout << "  AVISO: " << warning << '\n';
    if (!report.fixes.empty()) {
        std::cout << "Correcoes aplicadas: " << report.fixes.size() << '\n';
        for (const std::string &fix : report.fixes) std::cout << "  FIX: " << fix << '\n';
    }
    std::cout << "Status final: " << report.status << '\n';
}

void printConfigReport(const AppConfig &config)
{
    std::cout << "Configuracao do sistema\n";
    std::cout << "-----------------------\n";
    std::cout << "[system]\n"
              << "version=" << config.system.version << '\n'
              << "currency=" << config.system.currency << '\n'
              << "logs_enabled=" << (config.system.logsEnabled ? "true" : "false") << '\n'
              << "backup_enabled=" << (config.system.backupEnabled ? "true" : "false") << '\n'
              << "reports_enabled=" << (config.system.reportsEnabled ? "true" : "false") << '\n'
              << "debug_mode=" << (config.system.debugMode ? "true" : "false") << "\n\n"
              << "[paths]\n"
              << "data_dir=" << config.paths.dataDir << '\n'
              << "backup_dir=" << config.paths.backupDir << '\n'
              << "reports_dir=" << config.paths.reportsDir << "\n\n"
              << "[limits]\n"
              << "critical_autonomy_months=" << config.limits.criticalAutonomyMonths << '\n'
              << "warning_autonomy_months=" << config.limits.warningAutonomyMonths << '\n'
              << "expiration_critical_days=" << config.limits.expirationCriticalDays << '\n'
              << "expiration_warning_days=" << config.limits.expirationWarningDays << '\n';
}

void printErrorCatalog()
{
    std::cout << "Catalogo central de erros\n";
    std::cout << "-------------------------\n";
    std::cout << std::left << std::setw(24) << "codigo" << std::setw(38) << "descricao" << "acao recomendada\n";
    for (const ErrorCodeInfo &entry : ErrorHandler::catalog()) {
        std::cout << std::left << std::setw(24) << entry.code
                  << std::setw(38) << entry.description
                  << entry.recommendedAction << '\n';
    }
}
}

int SystemService::backup(int argc, char *argv[]) const
{
    const std::filesystem::path sourceRoot = argc >= 3 ? argv[2] : ".";
    const std::filesystem::path backupRoot = argc == 4 ? argv[3] : "backups";
    BackupManager manager;
    const BackupResult result = manager.createBackup(sourceRoot, backupRoot);
    if (!result.success) {
        std::cerr << "Erro ao criar backup: " << result.message << '\n';
        if (!result.backupPath.empty()) std::cerr << "Caminho do backup: " << result.backupPath.string() << '\n';
        return 1;
    }
    printBackupReport(result);
    return 0;
}

int SystemService::integrityCheck(int argc, char *argv[]) const
{
    DataIntegrityOptions options;
    if (argc >= 3 && std::string(argv[2]) == "--fix") {
        options.fix = true;
        if (argc == 4) options.rootPath = argv[3];
    } else if (argc == 3) {
        options.rootPath = argv[2];
    }
    DataIntegrityChecker checker;
    const DataIntegrityReport report = checker.check(options);
    printIntegrityReport(report);
    return report.success ? 0 : 1;
}

int SystemService::configShow(int argc, char *argv[]) const
{
    const std::filesystem::path configPath = argc == 3 ? argv[2] : kDefaultConfigPath;
    ConfigManager manager;
    const ConfigResult result = manager.loadOrCreate(configPath);
    if (!result.success) {
        const SystemError error = ErrorHandler::create("INVALID_CONFIG", ErrorSeverity::Error, "ConfigManager", result.message);
        std::cerr << "Erro [" << error.code << "]: " << error.message << '\n'
                  << "Sugestao: " << error.correctionSuggestion << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Configuracao central\n";
    std::cout << "Arquivo: " << configPath.string() << "\n\n";
    printConfigReport(result.config);
    return 0;
}

int SystemService::logsShow(int argc, char *argv[]) const
{
    const std::string levelFilter = argc == 3 ? argv[2] : "";
    if (!levelFilter.empty() && !Logger::isValidLevel(levelFilter)) {
        const SystemError error = ErrorHandler::create("INVALID_COMMAND", ErrorSeverity::Error, "Logger", "nivel de log invalido: " + levelFilter);
        std::cerr << "Erro [" << error.code << "]: " << error.message << '\n'
                  << "Sugestao: " << error.correctionSuggestion << '\n';
        return 1;
    }
    const std::filesystem::path logPath = Logger::logPath();
    const LogReadResult result = Logger::tail(logPath, 50, levelFilter);
    if (!result.success) {
        std::cerr << "Erro ao ler logs: " << result.message << '\n';
        return 1;
    }
    std::cout << "KAGE Home Supply - Logs operacionais\n";
    std::cout << "Arquivo: " << logPath.string() << '\n';
    if (!levelFilter.empty()) std::cout << "Filtro: " << levelFilter << '\n';
    std::cout << '\n';
    if (result.lines.empty()) {
        std::cout << "Nenhuma entrada de log encontrada.\n";
        return 0;
    }
    for (const std::string &line : result.lines) std::cout << line << '\n';
    return 0;
}

int SystemService::errorCodes(int, char *[]) const
{
    std::cout << "KAGE Home Supply - Codigos de erro\n\n";
    printErrorCatalog();
    return 0;
}
