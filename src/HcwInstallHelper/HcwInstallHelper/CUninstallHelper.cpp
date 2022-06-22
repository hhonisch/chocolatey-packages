#include "pch.h"
#include "CUninstallHelper.h"
#include "CInstallLog.h"
#include "Utils.h"

// Start uninstall
bool CUninstallHelper::Start(const std::wstring installDir)
{
    WriteOutput(L"Performing uninstall...\n");

    // Check for install log
    std::wstring installLogFilename;
    CombinePaths_FailOnError(installDir, L"" INSTALL_LOGFILE, installLogFilename);
    if (!PathFileExists(installLogFilename.c_str()))
    {
        CHECK_FAIL_MSG(FALSE, L"Error: Install log not found: %1", installLogFilename.c_str());
    }

    // Read all entries from install log
    std::vector<std::wstring> uninstallEntries;
    FILE* fUninstallLog = _wfsopen(installLogFilename.c_str(), L"r, ccs=UTF-8", _SH_DENYWR);
    CHECK_FAIL_MSG(fUninstallLog, "Error reading install log \"%1\": %2", installLogFilename.c_str(), GetCrtErrorMessage(errno).c_str());

    // Store file handle in smart pointer which will auto-close on destroy
    auto installLog_RW = MakeResourceWrapperFILE(*fUninstallLog);
    const int lineBufferLength = 2048;
    wchar_t lineBuffer[lineBufferLength];

    while (fgetws(lineBuffer, lineBufferLength, fUninstallLog))
    {
        // Make sure there is a terminating \0, otherwise the buffer would have been too small
        CHECK_FAIL_MSG(wcsnlen(lineBuffer, lineBufferLength) < lineBufferLength, "Error: Line buffer overflow");

        // Store non-empty lines in buffer
        std::wstring line = TrimString(lineBuffer);
        if (line.length() > 0)
        {
            uninstallEntries.push_back(line);
        }
    }

    // Close file handle
    installLog_RW.reset();

    // Process uninstall entries
    for (auto it = uninstallEntries.rbegin(); it != uninstallEntries.rend(); it++)
    {
        auto elements = SplitString(*it, '|');
        if (elements[0] == L"CreateFile")
        {
            UninstallRemoveFile(installDir, elements[1]);
        }
        else if (elements[0] == L"CreateDir")
        {
            UninstallRemoveDir(installDir, elements[1]);
        }
        else if (elements[0] == L"CreateRegKey")
        {
            UninstallRemoveRegKey(elements[1]);
        }
        else
        {
            WriteOutputFormatted(L"  Warning: Unknown uninstall entry: %1\n", it->c_str());
        }
    }

    // Done
    WriteOutput(L"Done performing uninstall\n");
    return true;
}

// Get full path to file / dir
std::wstring CUninstallHelper::GetFullFilePath(std::wstring installDir, std::wstring fileName)
{
    if (StringStartsWith(fileName, L".\\"))
    {
        std::wstring result;
        if (CombinePaths(installDir, &fileName[2], result))
        {
            return result;
        }
        else
        {
            return fileName;
        }
    }
    else
    {
        return fileName;
    }
}

// Remove registry key (if empty)
void CUninstallHelper::UninstallRemoveRegKey(const std::wstring regKeyPath)
{
    HKEY hkSubKey = 0;
    std::wstring regSubKeyPath;
    if (StringStartsWith(regKeyPath, L"HKLM32\\"))
    {
        regSubKeyPath = regKeyPath.substr(7);
        LSTATUS status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, regSubKeyPath.c_str(), 0, 0, 0, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, 0, &hkSubKey, 0);
        if (status != ERROR_SUCCESS)
        {
            return;
        }
        RegCloseKey(hkSubKey);

        WriteOutputFormatted(L"  Removing registry key %1...", regKeyPath.c_str());
        status = RegDeleteKey(HKEY_LOCAL_MACHINE, regSubKeyPath.c_str());
        if (status == ERROR_SUCCESS)
        {
            WriteOutput(L"done\n");
        }
        else
        {
            WriteOutputFormatted(L"\n    WARNING: Failed: %1\n", GetWin32ErrorMessage(status).c_str());
        }
    }
}

// Remove dir (if empty)
void CUninstallHelper::UninstallRemoveDir(const std::wstring installDir, const std::wstring dirName)
{
    std::wstring fullDirName = GetFullFilePath(installDir, dirName);

    // if directory does not exist, exit
    if (!DirExists(fullDirName.c_str()))
    {
        return;
    }

    // try to remove dir
    WriteOutputFormatted(L"  Removing directory %1...", fullDirName.c_str());

    // If delete fails, write warning
    if (RemoveDirectory(fullDirName.c_str()))
    {
        WriteOutput(L"done\n");
    }
    else
    {
        WriteOutputFormatted(L"\n    WARNING: Failed: %1\n", GetLastWin32ErrorMessage().c_str());
    }
}

// Remove file
void CUninstallHelper::UninstallRemoveFile(const std::wstring installDir, const std::wstring fileName)
{
    std::wstring fullFileName = GetFullFilePath(installDir, fileName);

    // if file does not exist, exit
    if (!PathFileExists(fullFileName.c_str()))
    {
        return;
    }

    // try to remove file
    WriteOutputFormatted(L"  Removing file %1...", fullFileName.c_str());
    SetFileAttributes(fullFileName.c_str(), FILE_ATTRIBUTE_NORMAL);

    // If delete fails, write warning
    if (DeleteFile(fullFileName.c_str()))
    {
        WriteOutput(L"done\n");
    }
    else
    {
        WriteOutputFormatted(L"\n    WARNING: Failed: %1\n", GetLastWin32ErrorMessage().c_str());
    }
}