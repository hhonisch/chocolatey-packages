using System;
using System.Runtime.InteropServices;
using System.Text;

namespace HcwInstallHelper
{
    public class HelperUtils
    {
        [DllImport("shell32", CharSet = CharSet.Unicode)]
        private static extern int SHGetSpecialFolderPath(IntPtr hwndOwner, StringBuilder lpszPath, int nFolder, int fCreate);

        // Get special folder path
        public static string GetSpecialFolderPath(int csidl)
        {
            const int capacity = 260;
            StringBuilder temp = new StringBuilder(capacity);
            int i = SHGetSpecialFolderPath(IntPtr.Zero, temp, csidl, 0);
            if (i != 0)
            {
                return temp.ToString();
            }
            else
            {
                return null;
            }
        }
    }

}
