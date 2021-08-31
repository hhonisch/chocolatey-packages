using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;

namespace HcwInstallHelper
{
    // Perform Hcw Install
    public class InstallHelper
    {
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

            // Create install log
            InstallLog installLog = new InstallLog(Path.Combine(installDir, Constants.INSTALL_LOGFILE));
            installLog.LogCreatedDir(installDir);
            installLog.LogCreatedFile($".\\{Constants.INSTALL_LOGFILE}");

            // Copy plain files to install dir
            CopyPlainFiles(sourceDir, installDirFull, installLog);

            // Extract files from MVA archives
            ExtractFilesFromArchives(sourceDir, installDirFull, installLog);

            // Create registry keys
            CreateRegistryKeys(installDirFull, installLog);

            // Create start menu entries
            CreateStartMenuEntries(installDirFull, installerIni, installLog);

            // Done
            Console.WriteLine("Done performing install");
            return 0;
        }


        // Create start menu entries
        private static void CreateStartMenuEntries(string installDir, IniFile installerIni, InstallLog installLog)
        {
            Console.WriteLine("Creating start menu entries...");

            // Create start menu folder
            string startMenuFolder = Path.Combine(HelperUtils.GetSpecialFolderPath(HelperUtils.CSIDL.CSIDL_COMMON_PROGRAMS), Constants.START_MENU_GROUP);
            installLog.LogCreatedDir(startMenuFolder);
            if (!Directory.Exists(startMenuFolder))
            {
                Console.WriteLine($"  Creating {startMenuFolder}...");
                Directory.CreateDirectory(startMenuFolder);
            }

            // Create entries
            var shell = new IWshRuntimeLibrary.WshShell();

            string[] shortcutFileNames = { "hcw.exe", "hcw.hlp", "dbhe.exe" };

            foreach (var fileName in shortcutFileNames)
            {
                var shortcutParams = installerIni.GetString("targets", fileName, "").Split('|');
                var shortcutName = shortcutParams[3];
                var shortcutArguments = shortcutParams[4];
                var shortcutPath = Path.Combine(startMenuFolder, $"{shortcutName}.lnk");
                var shortcutTargetPath = Path.Combine(installDir, fileName);
                Console.WriteLine($"  Creating {shortcutPath}...");
                installLog.LogCreatedFile(shortcutPath);
                var shortcut = shell.CreateShortcut(shortcutPath);
                shortcut.TargetPath = shortcutTargetPath;
                shortcut.Arguments = shortcutArguments;
                shortcut.Description = shortcutName;
                shortcut.Save();
            }
        }


        // Create registry keys
        private static void CreateRegistryKeys(string installDir, InstallLog installLog)
        {
            Console.WriteLine("Creating registry keys...");

            // Create app path entries
            using (var regHklm32 = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32))
            using (var regAppPathsBase = regHklm32.OpenSubKey(Constants.REG_PATH_APP_PATHS, true))
            {
                string[] appPathFileNames = { "hcw.exe", "hcrtf.exe", "dbhe.exe" };
                foreach (var appPathFileName in appPathFileNames)
                {
                    RegistryKey regAppPath = null;
                    try
                    {
                        installLog.LogCreatedRegKey($"HKLM32\\{Constants.REG_PATH_APP_PATHS}\\{appPathFileName}");
                        // Open / create key
                        regAppPath = regAppPathsBase.OpenSubKey(appPathFileName, true);
                        if (regAppPath == null)
                        {
                            regAppPath = regAppPathsBase.CreateSubKey(appPathFileName);
                        }
                        // Set full exe path
                        regAppPath.SetValue("", Path.Combine(installDir, appPathFileName));
                        // Set path
                        regAppPath.SetValue("Path", HelperUtils.EnsureTrailingPathDelimiter(installDir));
                    }
                    finally
                    {
                        // Cleanup
                        if (regAppPath != null)
                        {
                            regAppPath.Close();
                        }
                    }
                }
            }
        }


        // Extract files from MVA archives
        private static void ExtractFilesFromArchives(string sourceDir, string installDir, InstallLog installLog)
        {
            Console.WriteLine($"Extract files from archives...");
            string[] archiveNames = { "hcw.mva", "dbhe.mva", "Graphics.mva" };

            var archiveExtractor = new MvaArchiveExtractor();
            archiveExtractor.BeforeFileExtract += (sender, eventArgs) =>
                {
                    var pureFileName = Path.GetFileName(eventArgs.ArchiveItemHeader.FileName);
                    Console.WriteLine($"    Extracting archive item {pureFileName}...");
                    installLog.LogCreatedFile($".\\{pureFileName}");
                };

            foreach (var archiveName in archiveNames)
            {
                // Extract archive
                var archivePath = Path.Combine(sourceDir, $"{archiveName}");
                Console.WriteLine($"  Extracting archive {archivePath} to {installDir}");
                archiveExtractor.ExtractArchive(archivePath, installDir);
            }
        }


        // Copy plain files to install dir
        private static void CopyPlainFiles(string sourceDir, string installDir, InstallLog installLog)
        {
            Console.WriteLine($"Copy files...");
            // Copy files
            string[] filesToCopy = { "license.txt" };
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
                installLog.LogCreatedFile($".\\{fileName}");
                File.Copy(sourceFileName, destFileName, true);
            }
        }
    }
}
