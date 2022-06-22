#pragma once

class CInstallLog;
class CIniFile;

// Perform install actions
class CInstallHelper
{
private:
    // Copy plain files to install dir
    static void CopyPlainFiles(const std::wstring sourceDir, const std::wstring installDir, const CInstallLog& installLog);

    // Extract files from MVA archives
    static void ExtractFilesFromArchives(const std::wstring sourceDir, const std::wstring installDir, const CInstallLog& installLog);

    // Create registry keys
    static void CreateRegistryKeys(const std::wstring installDir, const CInstallLog& installLog);

    // Create start menu entries
    static void CreateStartMenuEntries(const std::wstring installDir, const CIniFile& installerIni, const CInstallLog& installLog);

public:
    // Start install
    static bool Start(const std::wstring sourceDir, const std::wstring installDir);
};
