#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct BackupResult {
    bool success = false;
    std::string message;
    std::filesystem::path backupPath;
    std::vector<std::string> copiedFiles;
    std::vector<std::string> ignoredFiles;
};

class BackupManager {
public:
    BackupResult createBackup(
        const std::filesystem::path &sourceRoot,
        const std::filesystem::path &backupRoot) const;

private:
    static std::string timestamp();
    static bool copyIfExists(
        const std::filesystem::path &source,
        const std::filesystem::path &target,
        BackupResult &result);
};
