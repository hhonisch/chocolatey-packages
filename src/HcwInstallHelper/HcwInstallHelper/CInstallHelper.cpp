#include "pch.h"
#include "CInstallHelper.h"
#include "CIniFile.h"
#include "CInstallLog.h"
#include "CMvaArchiveExtractor.h"
#include "Utils.h"

// Start install process
bool CInstallHelper::Start(const std::wstring sourceDir, const std::wstring installDir)
{
    WriteOutput(L"Performing install...\n");

    // Check source dir
    CHECK_FAIL_MSG(PathIsDirectory(sourceDir.c_str()), "Error: Directory not found: \"%1\"\n", sourceDir.c_str());

    // Create install dir
    if (!PathIsDirectory(installDir.c_str()))
    {
        WriteOutputFormatted(L"Creating install dir: %1\n", installDir.c_str());
        CreateDirectoryDeep_FailOnError(installDir.c_str());
    }

    // Get absolute path
    std::wstring installDirFull;
    GetFullPath_FailOnError(installDir, installDirFull);

    // Load installer ini
    std::wstring installerIniPath;
    CombinePaths_FailOnError(sourceDir, L"" INSTALLER_INI_NAME, installerIniPath);
    CIniFile installerIni(installerIniPath);

    // Create install log
    std::wstring installLogPath(installDir);
    CombinePaths_FailOnError(installDir, L"" INSTALL_LOGFILE, installLogPath);

    CInstallLog installLog(installLogPath.c_str());
    installLog.LogCreatedDir(installDir);
    installLog.LogCreatedFile(L".\\"  L"" INSTALL_LOGFILE);

    // Copy plain files to install dir
    CopyPlainFiles(sourceDir, installDirFull, installLog);

    // Extract files from MVA archives
    ExtractFilesFromArchives(sourceDir, installDirFull, installLog);

    // Create registry keys
    CreateRegistryKeys(installDirFull, installLog);

    // Create start menu entries
    CreateStartMenuEntries(installDirFull, installerIni, installLog);

    // Done
    WriteOutput(L"Done performing install\n");
    return true;
}

// Copy plain files to install dir
void CInstallHelper::CopyPlainFiles(const std::wstring sourceDir, const std::wstring installDir, const CInstallLog& installLog)
{
    WriteOutput(L"Copying files...\n");

    // Copy files
    const wchar_t* filesToCopy[] = { L"license.txt" };
    int filesCount = _countof(filesToCopy);

    for (int i = 0; i < filesCount; i++)
    {
        const wchar_t* filename = filesToCopy[i];
        WriteOutputFormatted(L"  Copying %1 to %2...", filename, installDir.c_str());

        // Get absolute file names
        std::wstring destFileName;
        std::wstring sourceFileName;
        CombinePaths_FailOnError(installDir, filename, destFileName);
        CombinePaths_FailOnError(sourceDir, filename, sourceFileName);

        // If dest file exists, make sure it's not readonly / system / hidden etc.
        if (PathFileExists(destFileName.c_str()))
        {
            SetFileAttributes_FailOnError(destFileName.c_str(), FILE_ATTRIBUTE_NORMAL);
        }

        // Copy file
        installLog.LogCreatedFile(std::wstring(L".\\") + filename);
        CopyFile_FailOnError(sourceFileName.c_str(), destFileName.c_str(), FALSE);
        WriteOutput(L"done\n");
    }
}

// Extract files from MVA archives
void CInstallHelper::ExtractFilesFromArchives(const std::wstring sourceDir, const std::wstring installDir, const CInstallLog& installLog)
{
    WriteOutput(L"Extracting files from archives...\n");
    const wchar_t* archiveNames[] = { L"hcw.mva", L"dbhe.mva", L"Graphics.mva" };
    int archivesCount = _countof(archiveNames);

    CMvaArchiveExtractor archiveExtractor;

    // Set callbacks
    archiveExtractor.SetCallbackBeforeFileExtract([&](const std::wstring& srcFileName, const std::wstring& destFileName)
    {
        WriteOutputFormatted(L"    Extracting archive item %1...", srcFileName.c_str());
        installLog.LogCreatedFile(std::wstring(L".\\") + srcFileName);
    });
    archiveExtractor.SetCallbackFileExtracted([&](const std::wstring& srcFileName, const std::wstring& destFileName)
    {
        WriteOutput(L"done\n");
    });

    // Start extracting
    for (int i = 0; i < archivesCount; i++)
    {
        // Extract archive
        const wchar_t* archiveName = archiveNames[i];
        std::wstring archivePath;
        CombinePaths_FailOnError(sourceDir, archiveName, archivePath);
        WriteOutputFormatted(L"  Extracting archive %1 to %2\n", archivePath.c_str(), installDir.c_str());
        archiveExtractor.ExtractArchive(archivePath, installDir);
    }
}

// Create registry keys
void CInstallHelper::CreateRegistryKeys(const std::wstring installDir, const CInstallLog& installLog)
{
    WriteOutput(L"Creating registry keys...\n");

    // Create app path entries
    HKEY hkAppPathsBase = 0;
    LSTATUS status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"" REG_PATH_APP_PATHS, 0, 0, 0, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, 0, &hkAppPathsBase, 0);
    CHECK_FAIL_MSG(status == ERROR_SUCCESS, L"Error opening registry key %1:%2\n", L"" REG_PATH_APP_PATHS, GetWin32ErrorMessage(status).c_str());

    // Resource wrapper for hkAppPathsBase
    auto hkAppPathsBase_RW = MakeResourceWrapperHKEY(hkAppPathsBase);

    const wchar_t* appPathFileNames[] = { L"hcw.exe", L"hcrtf.exe", L"dbhe.exe" };
    int fileNamesCount = _countof(appPathFileNames);
    for (int i = 0; i < fileNamesCount; i++)
    {
        const wchar_t* appPathFileName = appPathFileNames[i];

        // Log creating registry keys
        std::wstring regKeyPath = FormatString(L"HKLM32\\%1\\%2", L"" REG_PATH_APP_PATHS, appPathFileName);
        WriteOutputFormatted(L"    Creating  %1...", regKeyPath.c_str());
        installLog.LogCreatedRegKey(regKeyPath);

        // Open/create sub key
        HKEY hkAppPath = 0;
        LSTATUS status = RegCreateKeyEx(hkAppPathsBase, appPathFileName, 0, 0, 0, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, 0, &hkAppPath, 0);
        CHECK_FAIL_MSG(status == ERROR_SUCCESS, L"\nError opening registry key %1:%2\n", appPathFileName, GetWin32ErrorMessage(status).c_str());

        // Resource wrapper for hkAppPath
        auto hkAppPath_RW = MakeResourceWrapperHKEY(hkAppPath);

        // Set full path to app
        std::wstring appPathFull;
        CombinePaths_FailOnError(installDir, appPathFileName, appPathFull);
        status = RegSetKeyValue(hkAppPath, 0, L"", REG_SZ, appPathFull.c_str(), (appPathFull.size() + 1) * sizeof(appPathFull[0]));
        CHECK_FAIL_MSG(status == ERROR_SUCCESS, L"\nError setting registry value");

        // Set install dir, ensure trailing backslash
        std::wstring installDirWithTrailingBackslash = installDir;
        if ((installDir.size() == 0) || (installDir[installDir.size() - 1] != L'\\'))
        {
            installDirWithTrailingBackslash += L'\\';
        }
        status = RegSetKeyValue(hkAppPath, 0, L"Path", REG_SZ, installDirWithTrailingBackslash.c_str(),
            (installDirWithTrailingBackslash.size() + 1) * sizeof(installDirWithTrailingBackslash[0]));
        CHECK_FAIL_MSG(status == ERROR_SUCCESS, L"\nError setting registry value");
        WriteOutput(L"done\n");
    }
}

// Create start menu entries
void CInstallHelper::CreateStartMenuEntries(const std::wstring installDir, const CIniFile& installerIni, const CInstallLog& installLog)
{
    WriteOutput(L"Creating start menu entries...\n");

    // Create start menu folder
    std::wstring startMenuFolder;
    GetKnownFolderPath_FailOnError(CSIDL_COMMON_PROGRAMS | CSIDL_FLAG_CREATE, startMenuFolder);
    CombinePaths_FailOnError(startMenuFolder, L"" START_MENU_GROUP, startMenuFolder);
    installLog.LogCreatedDir(startMenuFolder);
    CreateDirectoryDeep_FailOnError(startMenuFolder.c_str());

    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    if (!SUCCEEDED(hr))
    {
        SetLastWin32ErrorFromHRESULT(hr);
        CHECK_FAIL_MSG(false, "COM initialization error:%1", GetLastWin32ErrorMessage().c_str());
    }

    // Create start menu entries
    const wchar_t* shortcutFileNames[] = { L"hcw.exe", L"hcw.hlp", L"dbhe.exe" };

    int fileNamesCount = _countof(shortcutFileNames);
    for (int i = 0; i < fileNamesCount; i++)
    {
        const wchar_t* shortcutFileName = shortcutFileNames[i];

        // Parse data from installer ini
        std::wstring str = installerIni.ReadString(L"targets", shortcutFileName, L"");
        std::vector<std::wstring> shortcutParams = SplitString(str, '|');

        std::wstring shortcutName = shortcutParams[3];
        std::wstring shortcutArguments = shortcutParams[4];
        std::wstring shortcutPath;
        CombinePaths_FailOnError(startMenuFolder, shortcutName + L".lnk", shortcutPath);
        std::wstring shortcutTargetPath;
        CombinePaths_FailOnError(installDir, shortcutFileName, shortcutTargetPath);
        WriteOutputFormatted(L"  Creating %1...", shortcutPath.c_str());
        installLog.LogCreatedFile(shortcutPath);

        // Create start menu shortcut
        IShellLink* pShellLink = 0;
        IPersistFile* pPersistFile = 0;
        HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
        bool bSuccess = SUCCEEDED(hr);
        if (bSuccess)
        {
            hr = pShellLink->SetPath(shortcutTargetPath.c_str());
        }
        bSuccess = bSuccess && SUCCEEDED(hr);
        if (bSuccess)
        {
            hr = pShellLink->SetArguments(shortcutArguments.c_str());
        }
        bSuccess = bSuccess && SUCCEEDED(hr);
        if (bSuccess)
        {
            hr = pShellLink->SetDescription(shortcutName.c_str());
        }
        bSuccess = bSuccess && SUCCEEDED(hr);

        // Use the IPersistFile object to save the shell link */
        if (bSuccess)
        {
            hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
        }
        bSuccess = bSuccess && SUCCEEDED(hr);
        if (bSuccess)
        {
            hr = pPersistFile->Save(shortcutPath.c_str(), TRUE);
        }
        bSuccess = bSuccess && SUCCEEDED(hr);

        // Cleanup
        if (pPersistFile)
        {
            pPersistFile->Release();
        }
        if (pShellLink)
        {
            pShellLink->Release();
        }

        // Check for error
        if (!bSuccess)
        {
            SetLastWin32ErrorFromHRESULT(hr);
            CHECK_FAIL_MSG(false, "\nError creating start menu shortcut:%1", GetLastWin32ErrorMessage().c_str());
        }
        else
        {
            WriteOutput(L"done\n");
        }
    }
}