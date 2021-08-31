using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace HcwInstallHelper
{
    class InstallLog : IDisposable
    {
        // Log file
        private readonly StreamWriter logFile;

        // Constructor
        public InstallLog(string logFileName)
        {
            // Create logfile
            logFile = new StreamWriter(File.Open(logFileName, FileMode.Append, FileAccess.Write, FileShare.Read), Encoding.UTF8);
        }


        // Dispose
        public void Dispose()
        {
            logFile.Flush();
            logFile.Close();
        }


        // Log creation of file
        internal void LogCreatedFile(string fileName)
        {
            logFile.WriteLine($"CreateFile|{fileName}");
            logFile.Flush();
        }


        // Log creation of registry key
        internal void LogCreatedRegKey(string regKeyPath)
        {
            logFile.WriteLine($"CreateRegKey|{regKeyPath}");
            logFile.Flush();
        }

        internal void LogCreatedDir(string dirName)
        {
            logFile.WriteLine($"CreateDir|{dirName}");
            logFile.Flush();
        }
    }
}
