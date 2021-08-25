using System;
using System.Runtime.InteropServices;
using System.Text;

namespace HcwInstallHelper
{
    public class IniFile
    {
        // Path to ini file
        private readonly string path;

        [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool WritePrivateProfileString(string section, string key, string val, string filePath);

        [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool WritePrivateProfileSection(string section, string data, string filePath);

        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern int GetPrivateProfileString(string section, string key, string def, StringBuilder retVal,
            int size, string filePath);

        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern int GetPrivateProfileSectionNames(IntPtr returnBuffer, int size, string filePath);

        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern int GetPrivateProfileSection(string section, IntPtr returnBuffer, int size, string filePath);

        // Constructor
        public IniFile(string IniFileName)
        {
            path = IniFileName;
        }


        // Write string value
        public bool WriteString(string section, string key, string value)
        {
            return WritePrivateProfileString(section, key, value, this.path);
        }


        // Read string value
        public string GetString(string section, string key, string defaultValue)
        {
            const int capacity = 4096;
            StringBuilder temp = new StringBuilder(capacity);
            int i = GetPrivateProfileString(section, key, defaultValue, temp, capacity, this.path);
            temp.Length = i;
            return temp.ToString();
        }

        // Get section names
        public string[] GetSectionNames()
        {
            int MAX_BUFFER = 32767;
            IntPtr pReturnedString = Marshal.AllocCoTaskMem(MAX_BUFFER);
            int bytesReturned = GetPrivateProfileSectionNames(pReturnedString, MAX_BUFFER, this.path);
            if (bytesReturned == 0)
            {
                return null;
            }
            string local = Marshal.PtrToStringUni(pReturnedString, (int)bytesReturned).ToString();
            Marshal.FreeCoTaskMem(pReturnedString);
            return local.Substring(0, local.Length - 1).Split('\0');
        }

        // Get section
        public string[] GetSection(string section)
        {
            int MAX_BUFFER = 32767;
            IntPtr pReturnedString = Marshal.AllocCoTaskMem(MAX_BUFFER);
            int bytesReturned = GetPrivateProfileSection(section, pReturnedString, MAX_BUFFER, this.path);
            if (bytesReturned == 0)
            {
                return null;
            }
            string local = Marshal.PtrToStringUni(pReturnedString, (int)bytesReturned).ToString();
            Marshal.FreeCoTaskMem(pReturnedString);
            return local.Substring(0, local.Length - 1).Split('\0');
        }

        // Write section
        public bool WriteSection(string section, string[] lines)
        {
            string data = String.Join("\0", lines) + "\0";
            return WritePrivateProfileSection(section, data, this.path);
        }
    }
}
