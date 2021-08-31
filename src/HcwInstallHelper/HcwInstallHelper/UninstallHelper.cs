using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace HcwInstallHelper
{
    // Perform Hcw Uninstall
    public class UninstallHelper
    {
        // Start uninstall
        internal static int Start(string installDir)
        {
            Console.WriteLine("Performing uninstall...");

            // Check for install log
            var installLogFilename = Path.Combine(installDir, Constants.INSTALL_LOGFILE);
            if (!File.Exists(installLogFilename))
            {
                Console.WriteLine($"Error: Install log not found: {installLogFilename}");
                return 1;
            }

            // Process uninstall entries
            var uninstallEntries = File.ReadAllLines(installLogFilename);
            for (var i = uninstallEntries.Length - 1; i >= 0; i--)
            {
                var elements = uninstallEntries[i].Split('|');
                if (elements[0] == "CreateFile")
                {
                    UninstallRemoveFile(installDir, elements[1]);
                }
                else if (elements[0] == "CreateDir")
                {
                    UninstallRemoveDir(installDir, elements[1]);
                }
                else if (elements[0] == "CreateRegKey")
                {
                    UninstallRemoveRegKey(elements[1]);
                }
                else
                {
                    Console.WriteLine($"WARNING: Unknown uninstall entry: {uninstallEntries[i]}");
                }
            }

            // Done
            Console.WriteLine("Done performing uninstall");
            return 0;
        }


        // Remove registry key (if empty)
        private static void UninstallRemoveRegKey(string regKeyPath)
        {
            RegistryKey regRootKey = null;
            String regSubKeyPath;
            try
            {
                // Open root
                if (regKeyPath.StartsWith("HKLM32\\"))
                {
                    regRootKey = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32);
                    regSubKeyPath = regKeyPath.Substring(7);
                }
                else
                {
                    throw new NotImplementedException();
                }

                // if registry key does not exist, exit
                RegistryKey regSubKey = regRootKey.OpenSubKey(regSubKeyPath);
                if (regSubKey == null)
                {
                    return;
                }
                regSubKey.Close();

                Console.WriteLine($"  Removing registry key {regKeyPath}...");
                regRootKey.DeleteSubKey(regSubKeyPath);
            }
            catch (Exception e)
            {
                Console.WriteLine($"    WARNING: Failed: {e.Message}");
            }
            finally
            {
                regRootKey?.Close();
            }
        }


        // Remove dir (if empty)
        private static void UninstallRemoveDir(string installDir, string dirName)
        {
            var fullDirName = GetFullFilePath(installDir, dirName);

            // if directory does not exist, exit
            if (!Directory.Exists(fullDirName))
            {
                return;
            }

            // try to remove dir
            Console.WriteLine($"  Removing directory {fullDirName}...");
            try
            {
                Directory.Delete(fullDirName);
            }
            catch (Exception e)
            {
                Console.WriteLine($"    WARNING: Failed: {e.Message}");
            }
        }


        // Get full path to file / dir
        private static string GetFullFilePath(string installDir, string fileName)
        {
            if (fileName.StartsWith(".\\"))
            {
                return Path.Combine(installDir, fileName.Substring(2));
            }
            else
            {
                return fileName;
            }
        }


        // Remove file
        private static void UninstallRemoveFile(string installDir, string fileName)
        {
            var fullFileName = GetFullFilePath(installDir, fileName);

            // if file does not exist, exit
            if (!File.Exists(fullFileName))
            {
                return;
            }

            // try to remove file
            Console.WriteLine($"  Removing file {fullFileName}...");
            try
            {
                File.SetAttributes(fullFileName, FileAttributes.Normal);
                File.Delete(fullFileName);
            }
            catch (Exception e)
            {
                Console.WriteLine($"    WARNING: Failed: {e.Message}");
            }
        }
    }
}
