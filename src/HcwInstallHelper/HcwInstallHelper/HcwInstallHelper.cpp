#include "pch.h"
#include "CInstallHelper.h"
#include "CUninstallHelper.h"
#include "Utils.h"

// Print usage
void PrintUsage()
{
    WriteOutput(L"Install helper for Chocolatey package help - workshop\n");
    WriteOutput(L"Usage:\n");
    WriteOutput(L"HcwInstallHelper install <setup_files_dir> <install_dir>\n");
    WriteOutput(L"HcwInstallHelper uninstall <install_dir>\n");
}

// Main function
int wmain(int argc, wchar_t* argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    try
    {
        // Convert args to vector

        std::vector<std::wstring> arguments(argv + 1, argv + argc);

        // No args => print usage
        if (arguments.size() == 0)
        {
            PrintUsage();
            return 1;
        }

        // Install mode
        if ((arguments.size() == 3) && (arguments[0] == L"install"))
        {
            // Read params
            std::wstring sourceDir = TrimString(arguments[1]);
            std::wstring installDir = TrimString(arguments[2]);

            // simple validation
            if (installDir.empty() || sourceDir.empty())
            {
                PrintUsage();
                return 1;
            }

            // Params seem to be legit => start install
            return CInstallHelper::Start(sourceDir, installDir) ? 0 : 1;
        }

        // Uninstall mode
        if ((arguments.size() == 2) && (arguments[0] == L"uninstall"))
        {
            std::wstring installDir = TrimString(arguments[1]);

            // Params seem to be legit => start install
            return CUninstallHelper::Start(installDir) ? 0 : 1;
        }

        // Fallback
        PrintUsage();
        return 1;
    }
    catch (...)
    {
        // Return error on exception
        return 1;
    }
}