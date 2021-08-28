using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;

namespace HcwInstallHelper
{
    // Perform Hcw Install
    public class InstallHelper
    {
        // Uninstall registry path 32 bit and 64 bit
        private const string REG_PATH_UNINSTALL_BASE = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall";

        // Uninstall registry key
        private const string REG_KEY_UNINSTALL = "Help Workshop";

        // Uninstall display name
        private const string UNINSTALL_DISPLAY_NAME = "Help Workshop";

        // Name of start menu group
        private const string START_MENU_GROUP = "Microsoft Help Workshop";

        // Name of install logfile
        private const string INSTALL_LOGFILE = "Help Workshop.log";


        // Start install
        internal static int Start(string sourceDir, string installDir)
        {
            Console.WriteLine("Performing install...");

            // Check sourcedir
            if (!Directory.Exists(sourceDir))
            {
                Console.WriteLine($"Error: Dir not found: {sourceDir}");
                return 1;
            }

            // Check install dir
            if (!Directory.Exists(installDir))
            {
                Console.WriteLine($"Creating install dir: {installDir}");
                Directory.CreateDirectory(installDir);
            }
            var installDirFull = Path.GetFullPath(installDir);

            // Load installer ini
            IniFile installerIni = new IniFile(Path.Combine(sourceDir, "_instpgm.ini"));

            // Copy plain files to install dir
            CopyPlainFiles(sourceDir, installDirFull);

            // Extract files from MVA archives
            var mvaArchives = new List<MvaArchiveData>();
            mvaArchives.Add(new MvaArchiveData("hcw"));
            mvaArchives.Add(new MvaArchiveData("dbhe"));
            mvaArchives.Add(new MvaArchiveData("Graphics"));
            ExtractFilesFromArchives(sourceDir, installDirFull, mvaArchives);

            // Create registry keys
            CreateRegistryKeys(installDirFull);

            // Create start menu entries
            CreateStartMenuEntries(installDirFull, installerIni);

            // Create install log
            CreateInstallLog(installDirFull, installerIni, mvaArchives);

            Console.WriteLine("Done performing install");
            return 0;
        }


        // Create install log
        private static void CreateInstallLog(string installDir, IniFile installerIni, List<MvaArchiveData> archives)
        {
            var winDir = Environment.GetFolderPath(Environment.SpecialFolder.Windows);
            var setupLogFilename = Path.Combine(winDir, INSTALL_LOGFILE);
            Console.WriteLine("Creating install log {setupLogFilename}...");

            // Remove any existing install logs
            if (File.Exists(setupLogFilename))
            {
                Console.WriteLine("  Remove existing install log...");
                File.SetAttributes(setupLogFilename, FileAttributes.Normal);
                File.Delete(setupLogFilename);
            }

            // Setup log ini file
            Console.WriteLine("  Write install log...");
            IniFile setupLog = new IniFile(setupLogFilename);

            // Process section "install"
            var sectionLines = installerIni.GetSection("Install");
            foreach (var line in sectionLines)
            {
                var lineSplit = line.Split('=');
                setupLog.WriteString("Install", lineSplit[0], "0|1|0");
            }

            // Process archive files
            foreach (var archive in archives)
            {
                foreach (var fileName in archive.files)
                {
                    setupLog.WriteString(archive.archiveName, fileName, "0|1|0");
                }
            }

            // Process section "destinations"
            var v = installerIni.GetString("destinations", "0", "");
            setupLog.WriteString("destinations", "0", $"{v}|{installDir}\\");

            // Process section "groups"
            sectionLines = installerIni.GetSection("groups");
            setupLog.WriteSection("groups", sectionLines);

            // Process section "targets"
            sectionLines = installerIni.GetSection("targets");
            foreach (var line in sectionLines)
            {
                var lineSplit = line.Split('=');
                var fileName = lineSplit[0];
                var path = Path.Combine(installDir, fileName);
                setupLog.WriteString("targets", fileName, $"{lineSplit[1]}|{path}");
            }
        }


        // Create start menu entries
        private static void CreateStartMenuEntries(string installDir, IniFile installerIni)
        {
            Console.WriteLine("Creating start menu entries...");

            // Create start menu folder
            string startMenuFolder = Path.Combine(HelperUtils.GetSpecialFolderPath(HelperUtils.CSIDL.CSIDL_COMMON_PROGRAMS), START_MENU_GROUP);
            if (!Directory.Exists(startMenuFolder))
            {
                Console.WriteLine($"  Creating {startMenuFolder}...");
                Directory.CreateDirectory(startMenuFolder);
            }

            // Create entries
            var shell = new IWshRuntimeLibrary.WshShell();
            var sectionLines = installerIni.GetSection("targets");
            foreach (var line in sectionLines)
            {
                string[] v = line.Split('=');
                var fileName = v[0];
                var shortcutParams = v[1].Split('|');
                var shortcutName = shortcutParams[3];
                var shortcutArguments = shortcutParams[4];
                var shortcutPath = Path.Combine(startMenuFolder, $"{shortcutName}.lnk");
                var shortcutTargetPath = Path.Combine(installDir, fileName);
                Console.WriteLine($"  Creating {shortcutPath}...");
                var shortcut = shell.CreateShortcut(shortcutPath);
                shortcut.TargetPath = shortcutTargetPath;
                shortcut.Arguments = shortcutArguments;
                shortcut.Description = shortcutName;
                shortcut.Save();
            }
        }


        // Create registry keys
        private static void CreateRegistryKeys(string installDir)
        {
            Console.WriteLine("Creating registry keys...");
            Console.WriteLine("  Creating uninstall registry key...");

            // Create uninstall registry key using 32 bit registry
            using (var regHklm32 = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32))
            using (var regUninstallBase = regHklm32.OpenSubKey(REG_PATH_UNINSTALL_BASE, true))
            {
                RegistryKey regUninstall = null;
                try
                {
                    // Open / create uninstall key
                    regUninstall = regUninstallBase.OpenSubKey(REG_KEY_UNINSTALL, true);
                    if (regUninstall == null)
                    {
                        regUninstall = regUninstallBase.CreateSubKey(REG_KEY_UNINSTALL);
                    }
                    // Set values
                    regUninstall.SetValue("DisplayName", UNINSTALL_DISPLAY_NAME);
                    var uninstallExe = Path.Combine(installDir, "_instpgm.exe");
                    regUninstall.SetValue("UninstallString", $"\"{uninstallExe}\" /U");
                }
                finally
                {
                    // Cleanup
                    if (regUninstall != null)
                    {
                        regUninstall.Close();
                    }
                }
            }
        }


        // Extract files from MVA archives
        private static void ExtractFilesFromArchives(string sourceDir, string installDir, List<MvaArchiveData> archives)
        {
            Console.WriteLine($"Extract files from archives...");
            foreach (var archiveData in archives)
            {
                // Extract archive
                var archivePath = Path.Combine(sourceDir, $"{archiveData.archiveName}.mva");
                MvaArchiveExtractor.ExtractArchive(archivePath, installDir, archiveData.files, (msg) => { Console.WriteLine($"  {msg}"); });
            }
        }


        // Copy plain files to install dir
        private static void CopyPlainFiles(string sourceDir, string installDir)
        {
            Console.WriteLine($"Copy files...");
            // Copy files
            string[] filesToCopy = { "license.txt", "_instpgm.exe", "_iwdinst.exe" };
            foreach (string fileName in filesToCopy)
            {
                Console.WriteLine($"  Copying {fileName} to {installDir}");
                var destFileName = Path.Combine(installDir, fileName);
                var sourceFileName = Path.Combine(sourceDir, fileName);

                // If file exists, make sure it's not readonly / system / hidden etc.
                if (File.Exists(destFileName))
                {
                    File.SetAttributes(destFileName, FileAttributes.Normal);
                }
                // Copy file
                File.Copy(sourceFileName, destFileName, true);
            }
        }


        // Contains data of extracted MVA archive
        private class MvaArchiveData
        {
            internal string archiveName;
            internal List<string> files;

            // Constructor 
            public MvaArchiveData(string archiveName)
            {
                this.archiveName = archiveName;
                this.files = new List<string>();
            }
        }
    }
}
