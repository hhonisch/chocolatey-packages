using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace HcwInstallHelper
{
    public class HelperUtils
    {
        [DllImport("shell32", CharSet = CharSet.Unicode)]
        private static extern int SHGetSpecialFolderPath(IntPtr hwndOwner, StringBuilder lpszPath, int nFolder, int fCreate);

        // Enumerare CSIDL values
        public enum CSIDL
        {
            CSIDL_COMMON_PROGRAMS = 0x17
        }

        // Get special folder path
        public static string GetSpecialFolderPath(CSIDL csidl)
        {
            const int capacity = 260;
            StringBuilder temp = new StringBuilder(capacity);
            int i = SHGetSpecialFolderPath(IntPtr.Zero, temp, (int)csidl, 0);
            if (i != 0)
            {
                return temp.ToString();
            }
            else
            {
                return null;
            }
        }


        // Ensure trailing path delimiter
        public static string EnsureTrailingPathDelimiter(string path)
        {
            if (path.Length > 0)
            {
                char lastChar = path[path.Length - 1];
                if ((lastChar == Path.DirectorySeparatorChar) || (lastChar == Path.AltDirectorySeparatorChar))
                {
                    return path;
                }
            }
            if (path.IndexOf(Path.AltDirectorySeparatorChar) >= 0)
            {
                return (path + Path.AltDirectorySeparatorChar);
            }
            else
            {
                return (path + Path.DirectorySeparatorChar);
            }

        }
    }

}
