using System;
using System.Collections.Generic;
using System.IO;

namespace HcwInstallHelper
{
    // Perform Hcw Install
    public class InstallHelper
    {
        // Installer ini
        private static IniFile installerIni;

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
                return 1;
            }

            // Load installer ini
            installerIni = new IniFile(Path.Combine(sourceDir, "_instpgm.ini"));

            // Copy plain files to install dir
            CopyPlainFiles(sourceDir, installDir);

            // Extract files from MVA archives
            var mvaArchives = new List<MvaArchiveData>();
            mvaArchives.Add(new MvaArchiveData("hcw"));
            mvaArchives.Add(new MvaArchiveData("dbhe"));
            mvaArchives.Add(new MvaArchiveData("Graphics"));
            foreach (var archiveData in mvaArchives)
            {
                // Extract archive
                var archivePath = Path.Combine(sourceDir, $"{archiveData.archiveName}.mva");
                MvaArchiveExtractor.ExtractArchive(archivePath, installDir, archiveData.files, (msg) => { Console.WriteLine(msg); });
            }
            return 0;
        }


        // Copy plain files to install dir
        private static void CopyPlainFiles(string sourceDir, string installDir)
        {
            // Copy files
            string[] filesToCopy = { "license.txt", "_instpgm.exe", "_iwdinst.exe" };
            foreach (string fileName in filesToCopy)
            {
                Console.WriteLine($"Copying {fileName} to {installDir}");
                var destFileName = Path.Combine(installDir, fileName);
                var sourceFileName = Path.Combine(sourceDir, fileName);

                // If file exists, make sure it's not readonly
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
