using System;

namespace HcwInstallHelper
{
    public class Program
    {
        // Enum for main command
        private enum MainCommand { mcInstall, mcUninstall };

        // Main command
        private static MainCommand mainCommand;

        // Source dir for installation
        private static string sourceDir;

        // Target install dir
        private static string installDir;

        // Main method
        static int Main(string[] args)
        {
            // Parse command line
            if (!ParseCommandLineArgs(args))
            {
                return 1;
            }

            // Perform install
            if (mainCommand == MainCommand.mcInstall)
            {
                return InstallHelper.Start(sourceDir, installDir);
            }


            // Perform uninstall
            if (mainCommand == MainCommand.mcUninstall)
            {
                return UninstallHelper.Start();
            }


            return 0;
        }


        // Parse command line argumsnts
        private static bool ParseCommandLineArgs(string[] args)
        {
            // No args => print usage
            if (args.Length == 0)
            {
                PrintUsage();
                return false;
            }

            // Install mode
            if ((args.Length == 3) && (args[0] == "install"))
            {
                // Read params
                sourceDir = args[1].Trim();
                installDir = args[2].Trim();
                // simple validation
                if (installDir == "" || sourceDir == "")
                {
                    PrintUsage();
                    return false;
                }
                // Params seem to be legit
                mainCommand = MainCommand.mcInstall;
                return true;
            }

            // Uninstall mode
            if ((args.Length == 1) && (args[0] == "uninstall"))
            {
                mainCommand = MainCommand.mcUninstall;
                return true;
            }

            // Fallback
            PrintUsage();
            return false;
        }


        // Print usage
        private static void PrintUsage()
        {
            Console.WriteLine("Install helper for Chocolatey package help-workshop\n");
            Console.WriteLine("Usage:");
            Console.WriteLine("HcwInstallHelper install <setup_files_dir> <install_dir>");
            Console.WriteLine("HcwInstallHelper uninstall");
        }
    }
}
