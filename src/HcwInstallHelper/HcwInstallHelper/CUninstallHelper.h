#pragma once

// Perform Uninstall actions
class CUninstallHelper
{
private:
    // Get full path to file / dir
    static std::wstring GetFullFilePath(std::wstring installDir, std::wstring fileName);

    // Remove registry key (if empty)
    static void UninstallRemoveRegKey(const std::wstring regKeyPath);

    // Remove dir (if empty)
    static void UninstallRemoveDir(const std::wstring installDir, const std::wstring dirName);

    // Remove file
    static void UninstallRemoveFile(const std::wstring installDir, const std::wstring fileName);

public:
    // Start uninstall
    static bool Start(const std::wstring installDir);
};
