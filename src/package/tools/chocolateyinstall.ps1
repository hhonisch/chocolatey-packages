$ErrorActionPreference = 'Stop'

#################
# C# code
#################
[string] $code = @'
using System;
using System.Runtime.InteropServices;
using System.Text;

public class ChecksumHelper
{
    // Compute Adler32 checksum
    public static byte[] ComputeAdler32Checksum(byte[] bytesArray, int byteStart, int bytesToRead)
    {
        uint checksum = 1;
        int n;
        uint s1 = checksum & 0xFFFF;
        uint s2 = checksum >> 16;

        while (bytesToRead > 0)
        {
            n = (3800 > bytesToRead) ? bytesToRead : 3800;
            bytesToRead -= n;

            while (--n >= 0)
            {
                s1 = s1 + (uint)(bytesArray[byteStart++] & 0xFF);
                s2 = s2 + s1;
            }

            s1 %= 65521;
            s2 %= 65521;
        }

        checksum = (s2 << 16) | s1;

        byte[] output = new byte[4];

        output[0] = (byte)(checksum >> 24);
        output[1] = (byte)(checksum >> 16);
        output[2] = (byte)(checksum >> 8);
        output[3] = (byte)(checksum);

        return output;
    }

    // CRC32 Lookup table
    private static readonly uint[] crctab =
    {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
        0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
        0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
        0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
        0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
        0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
        0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
        0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
        0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
        0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
        0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
        0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
        0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
        0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
        0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
        0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
        0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
        0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
        0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
        0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
        0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
        0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
        0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
        0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
        0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
        0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
        0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
        0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
        0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
        0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
        0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
        0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
        0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
        0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
        0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
        0x2d02ef8d
    };

    // Compute variant of CRC32 checksum
    public static byte[] ComputeCRC32XChecksum(byte[] Data, int byteStart, int bytesToRead)
    {
        uint crc = 0;
        for (int i = byteStart; i < (byteStart + bytesToRead); i++)
        {
            crc = ((crc >> 6) & 0xFFFFFF) ^ crctab[(crc & 0xff) ^ Data[i]];
        }
        byte[] output = new byte[4];

        output[3] = (byte)(crc >> 24);
        output[2] = (byte)(crc >> 16);
        output[1] = (byte)(crc >> 8);
        output[0] = (byte)(crc);

        return output;
    }

}

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

'@
if (-not ([System.Management.Automation.PSTypeName]'ChecksumHelper').Type) {
    Add-Type -TypeDefinition $code -Language CSharp
}


# Determine script root for PS 2
if ($PSVersionTable.PSVersion.Major -le 2) {
    [string] $_PSScriptRoot = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition
}
# Use built in variable for later PS versions
else {
    [string] $_PSScriptRoot = $PSScriptRoot
}



#####################
# Constant values
#####################

# Unix epoch start
[DateTime] $EPOCH_START = New-Object DateTime 1970, 1, 1, 0, 0, 0, 0, UTC

# Zlib headers
[array] $ZLIB_HEADERS = @(0x0178, 0x5e78, 0x9c78, 0xda78)

# Max path length
[UInt16] $MAX_PATH = 260

# MVA archive header size
[UInt16] $MVA_ARCHIVE_HEADER_SIZE = 8

# MVA magic number archive header
[UInt32] $MVA_MAGICNUM_ARCHIVE = 0x686C666D

# MVA magic number archive item header
[UInt32] $MVA_MAGICNUM_ARCHIVE_ITEM = 0x6E65666D

# MVA magic number archive item header 2
[UInt32] $MVA_MAGICNUM_ARCHIVE_ITEM_2 = 0x7ffdf000

# MVA magic number archive item header 3
[UInt32] $MVA_MAGICNUM_ARCHIVE_ITEM_3 = 0x00000004

# Adler32 checksum size
[byte] $ADLER32_CHECKSUM_SIZE = 4

# Size for CopySteam buffer
[UInt32] $COPYSTREAM_BUFFER_SIZE = 0x8000

# Uninstall registry path 32 bit and 64 bit
[string] $REG_PATH_UNINSTALL_32 = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Help Workshop'
[string] $REG_PATH_UNINSTALL_64 = 'HKLM:\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Help Workshop'

# Uninstall display name
[string] $UNINSTALL_DISPLAY_NAME = 'Help Workshop'

# Name of start menu group
[string] $START_MENU_GROUP = "Microsoft Help Workshop"

# Name of install folder
[string] $INSTALL_FOLDER = "Help Workshop"

# Name of install logfile
[string] $INSTALL_LOGFILE = "Help Workshop.log"



#####################
# Global variables
#####################

# Buffer for CopyStream
[byte[]] $g_CopySteamBuffer = $null



#####################
# Functions
#####################


# Copy bytes from stream to stream
function CopyStream([System.IO.Stream] $in, [System.IO.Stream] $out, [int] $numBytes) {
    # Lazy initialize buffer
    if ($mull -eq $g_CopySteamBuffer) {
        $g_CopySteamBuffer = New-Object byte[]($COPYSTREAM_BUFFER_SIZE)
    }
    if ($numBytes -eq 0) {
        $numBytes = [int]::MaxValue
    }
    [int] $numBytesRead = 0
    while ($numBytes -gt 0) {
        $numBytesRead = $in.Read($g_CopySteamBuffer, 0, [System.Math]::Min($g_CopySteamBuffer.Length, $numBytes))
        if ($numBytesRead -gt 0) {
            $out.Write($g_CopySteamBuffer, 0, $numBytesRead)
            $numBytes -= $numBytesRead
        }
        else {
            break
        }
    }
}



# Read Mva header
function MvaReadArchiveHeader([System.IO.BinaryReader] $binReader) {

    # Save current stream position
    [UInt32] $streamPosStart = $binReader.BaseStream.Position

    # Read magic number
    [UInt32] $magicNumber = $binReader.ReadUInt32()
    if ($magicNumber -ne $MVA_MAGICNUM_ARCHIVE) {
        throw "Error: Invalid magic number in archive header"
    }

    # Read version
    [UInt16] $version = $binReader.ReadUInt16()
    if ($version -ne 1) {
        throw "Error: Unsupported version in archive header"
    }

    # Skip to end of header
    [UInt32] $bytesToSkip = $MVA_ARCHIVE_HEADER_SIZE + $streamPosStart - $binReader.BaseStream.Position
    $binReader.BaseStream.Seek($bytesToSkip, [System.IO.SeekOrigin]::Current) | Out-Null
}



# Read Mva archive item header
function MvaReadArchiveItemHeader([System.IO.BinaryReader] $binReader, [ref] $headerInfo) {

    # Save current stream position
    [UInt32] $streamPosStart = $binReader.BaseStream.Position

    # Read magic number
    [UInt32] $magicNumber = $binReader.ReadUInt32()
    if ($magicNumber -ne $MVA_MAGICNUM_ARCHIVE_ITEM) {
        throw "Error: Invalid magic number in archive item header"
    }

    # Read version
    [UInt16] $version = $binReader.ReadUInt16()
    if ($version -ne 1) {
        throw "Error: Unsupported version in archive item header"
    }

    # data offset
    [UInt16] $offsetItemData = $binReader.ReadUInt16()

    # file times
    [UInt32] $timeLastModified = $binReader.ReadUInt32()
    [UInt32] $timeLastAccessed = $binReader.ReadUInt32()

    # file name
    [String] $filename = [System.Text.Encoding]::ASCII.GetString($binReader.ReadBytes($MAX_PATH)).TrimEnd([char] 0x0000)

    # sizes
    [UInt32] $sizeUncompressed = $binReader.ReadUInt32()
    [UInt32] $sizeCompressed = $binReader.ReadUInt32()

    # CRC32 checksum
    [UInt32] $crc32Checksum = $binReader.ReadUInt32()

    # Read magic number(2)
    [UInt32] $magicNumber2 = $binReader.ReadUInt32()
    if ($magicNumber2 -ne $MVA_MAGICNUM_ARCHIVE_ITEM_2) {
        throw "Error: Invalid magic number (2) in archive item header"
    }

    # Read magic number(3)
    [UInt32] $magicNumber3 = $binReader.ReadUInt32()
    if ($magicNumber3 -ne $MVA_MAGICNUM_ARCHIVE_ITEM_3) {
        throw "Error: Invalid magic number (3) in archive item header"
    }

    # Skip to end of header
    [UInt32] $bytesToSkip = $offsetItemData + $streamPosStart - $binReader.BaseStream.Position
    $binReader.BaseStream.Seek($bytesToSkip, [System.IO.SeekOrigin]::Current) | Out-Null

    # Store and return header info
    $headerInfo.Value = @{
        FileName         = $filename
        LastModified     = $EPOCH_START.AddSeconds($timeLastModified)
        LastAccessed     = $EPOCH_START.AddSeconds($timeLastAccessed)
        SizeCompressed   = $sizeCompressed
        SizeUncompressed = $sizeUncompressed
        Crc32Checksum    = $crc32Checksum
    }
}



# Extract Mva archive item
function MvaExtractArchiveItem([System.IO.BinaryReader] $binReader, [hashtable] $itemHeader, [string] $destFileName) {
    Write-Host "  Extracting archive item $destFileName..."

    # Save current stream position
    [UInt32] $streamPosStart = $binReader.BaseStream.Position

    # Read Zlib header
    [UInt16] $zlibHeader = $binReader.ReadUInt16()
    if ($ZLIB_HEADERS -NotContains $zlibHeader) {
        throw "Error: Invalid zlib header"
    }

    # Read compressed content
    [UInt32] $bytesToRead = $itemHeader.sizeCompressed - $binReader.BaseStream.Position + $streamPosStart - $ADLER32_CHECKSUM_SIZE
    [System.IO.Stream] $msCompressedContent = New-Object System.IO.MemoryStream($bytesToRead)
    CopyStream $binReader.BaseStream $msCompressedContent $bytesToRead

    # Read Adler32 checksum from archive
    [UInt32] $adler32Checksum = $binReader.ReadUInt32()

    # Get uncompressed content
    [System.IO.Stream] $msUncompressedContent = New-Object System.IO.MemoryStream($itemHeader.sizeUncompressed)
    $msCompressedContent.Position = 0
    [System.IO.Stream] $deflateStream = New-Object System.IO.Compression.DeflateStream($msCompressedContent, [System.IO.Compression.CompressionMode]::Decompress, $true)
    try {
        CopyStream $deflateStream $msUncompressedContent
    }
    finally {
        $deflateStream.Close()
    }

    # Get content as byte buffer
    $msUncompressedContent.Position = 0
    [byte[]] $buffer = $msUncompressedContent.GetBuffer()


    # Verify Adler-32 checksum
    [byte[]] $adler32Bytes = [ChecksumHelper]::ComputeAdler32Checksum($buffer, 0, $buffer.Length)
    [UInt32] $computedAdler32Checksum = [System.BitConverter]::ToUInt32($adler32Bytes, 0)
    if ($computedAdler32Checksum -ne $adler32Checksum) {
        throw "Error: Adler32 checksum mismatch"
    }

    # Verify CRC checksum
    [byte[]] $crc32Bytes = [ChecksumHelper]::ComputeCRC32XChecksum($buffer, 0, $buffer.Length)
    [UInt32] $computedCrc32Checksum = [System.BitConverter]::ToUInt32($crc32Bytes, 0)
    if ($computedCrc32Checksum -ne $itemHeader.Crc32Checksum) {
        throw "Error: CRC32 checksum mismatch"
    }

    # Write uncompressed content to file
    $msUncompressedContent.Position = 0
    $fileStreamOut = New-Object System.IO.FileStream($destFileName, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write)
    try {
        CopyStream $msUncompressedContent $fileStreamOut
    }
    finally {
        $fileStreamOut.Close()
    }

    # Set file last write time
    [System.IO.File]::SetLastWriteTimeUtc($destFileName, $itemHeader.LastModified)
}



# Extract Mva archive
function MvaExtractArchive([string] $mvaFileName, [string] $destDir, [object] $filesExtracted) {
    Write-Host "Extracting archive $mvaFileName to $destDir"

    [System.IO.Stream] $fileStreamIn = $null
    [System.IO.Stream] $bufferedStreamIn = $null
    try {
        # Open archive file
        $fileStreamIn = New-Object System.IO.FileStream($mvaFileName, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
        $bufferedStreamIn = New-Object System.IO.BufferedStream($fileStreamIn)
        [System.IO.BinaryReader] $binReader = New-Object System.IO.BinaryReader($bufferedStreamIn)

        # Read archive header info
        MvaReadArchiveHeader $binReader
        [hashtable] $mvaItemHeader = $null

        # Loop through archive items
        while ($binReader.PeekChar() -ne -1) {
            # Read archive item header
            MvaReadArchiveItemHeader $binReader ([ref] $mvaItemHeader)

            # Build destination file name
            [string] $pureFileName = [System.IO.Path]::GetFileName($mvaItemHeader.FileName)
            [string] $destFileName = [System.IO.Path]::Combine($destDir, $pureFileName)
            $filesExtracted.Add($pureFileName)

            # Extract archive item
            MvaExtractArchiveItem $binReader $mvaItemHeader $destFileName
        }

    }
    finally {
        if ($null -ne $bufferedStreamIn) {
            $bufferedStreamIn.Close()
        }
        if ($null -ne $fileStreamIn) {
            $fileStreamIn.Close()
        }
    }
}



# Create uninstall registry keys
function CreateUninstalRegKeys([string] $installDir) {

    # Create uninstall registry keys
    Write-Host "###############################"
    Write-Host "Creating uninstall registry keys"
    if ($env:PROCESSOR_ARCHITECTURE -eq "AMD64") {
        [string] $regPath = $REG_PATH_UNINSTALL_64
    }
    else {
        [string] $regPath = $REG_PATH_UNINSTALL_32
    }
    New-Item -Path $regPath -Force | Out-Null
    New-ItemProperty -Path $regPath -Name "DisplayName" -Value $UNINSTALL_DISPLAY_NAME -Force | Out-Null
    [string] $unstallExe = Join-Path $installDir "_instpgm.exe"
    New-ItemProperty -Path $regPath -Name "UninstallString" -Value "$unstallExe /U" -Force | Out-Null
}



# Create start menu entries
function CreateStartMenuEntries([string] $installDir, [IniFile] $installerIni) {

    # Create start menu entries
    Write-Host "###############################"
    Write-Host "Creating start menu entries"

    # Create start menu folder
    [string] $startMenuFolder = Join-Path ([HelperUtils]::GetSpecialFolderPath(0x17)) $START_MENU_GROUP
    if (-not (Test-Path $startMenuFolder)) {
        MkDir $startMenuFolder -Force | Out-Null
    }

    # Create entries
    [object] $objShell = New-Object -ComObject "WScript.Shell"
    [string[]] $sectionLines = $installerIni.GetSection("targets")
    foreach ($line in $sectionLines) {
        $v = $line -Split '='
        $fileName = $v[0]
        $shortcutParams = $v[1] -Split '\|'
        [string] $shortcutName = $shortcutParams[3]
        [string] $shortcutArguments = $shortcutParams[4]
        [string] $shortcutPath = Join-Path $startMenuFolder "$shortcutName.lnk"
        [string] $shortcutTargetPath = Join-Path $installDir $fileName
        [object] $shortcut = $objShell.CreateShortcut($shortcutPath)
        $shortcut.TargetPath = $shortcutTargetPath
        $shortcut.Arguments = $shortcutArguments
        $shortcut.Description = $shortcutName
        $shortcut.Save()
    }
}



# Create install log
function CreateInstallLog([string] $installDir, [IniFile] $installerIni, [hashtable] $archiveData) {

    # Write install log
    Write-Host "###############################"
    [string] $setupLogFilename = Join-Path $env:windir $INSTALL_LOGFILE
    Write-Host "Creating $setupLogFilename"

    # Remove any existing install logs
    if (Test-Path $setupLogFilename) {
        Remove-Item $setupLogFilename -Force
    }

    # Target log ini file
    [IniFile] $setupLog = New-Object IniFile($setupLogFilename)

    # Process section "install"
    [string[]] $sectionLines = $installerIni.GetSection("Install")
    foreach ($line in $sectionLines) {
        $v = $line -Split '='
        $setupLog.WriteString("Install", $v[0], "0|1|0") | Out-Null
    }

    # Process archive data
    foreach ($archiveName in @("hcw"; "dbhe"; "Graphics" )) {
        foreach ($fileName in $archiveData[$archiveName]) {
            $setupLog.WriteString($archiveName, $fileName, "0|1|0") | Out-Null
        }
    }

    # Process section "destinations"
    $v = $installerIni.GetString("destinations", "0", "")
    $setupLog.WriteString("destinations", "0", "$v|$installDir\") | Out-Null

    # Process section "groups"
    [string[]] $sectionLines = $installerIni.GetSection("groups")
    $setupLog.WriteSection("groups", $sectionLines) | Out-Null

    # Process section "targets"
    [string[]] $sectionLines = $installerIni.GetSection("targets")
    foreach ($line in $sectionLines) {
        $v = $line -Split '='
        $fileName = $v[0]
        $path = Join-Path $installDir $fileName
        $setupLog.WriteString("targets", $fileName, ($v[1] + "|" + $path)) | Out-Null
    }
}



# Perform installation
function DoInstall([string] $hcSetupExe, [string] $installDir, [string] $tmpDir) {

    # Extract CAB data from setup
    Write-Host "###############################"
    Write-Host "Extracting $hcSetupExe"
    Get-ChocolateyUnzip -FileFullPath $hcSetupExe -Destination $tmpDir | Out-Null

    # Installer ini file
    [IniFile] $installerIni = New-Object IniFile(Join-Path $tmpDir "_instpgm.ini")

    # Check for install dir
    if (-not (Test-Path $installDir)) {
        MkDir $installDir -Force | Out-Null
    }

    # Copy files to install dir
    Write-Host "###############################"
    Write-Host "Copying files"
    foreach ($filename in @("license.txt" , "_instpgm.exe" , "_iwdinst.exe") ) {
        Write-Host "  Copy $filename to $installDir"
        Copy-Item (Join-Path $tmpDir $filename)  (Join-Path $installDir $filename) -Force
    }

    # Extract files from MVA archives
    Write-Host "###############################"
    [hashtable] $archiveData = @{
        "hcw"      = New-Object 'System.Collections.Generic.List[string]'
        "dbhe"     = New-Object 'System.Collections.Generic.List[string]'
        "Graphics" = New-Object 'System.Collections.Generic.List[string]'
    }

    foreach ($item in $archiveData.GetEnumerator()) {
        MvaExtractArchive (Join-Path $tmpDir "$($item.Name).mva") $installDir $item.Value
    }

    # Create uninstall registry keys
    CreateUninstalRegKeys $installDir

    # Create start menu entries
    CreateStartMenuEntries $installDir $installerIni

    # Create install log
    CreateInstallLog $installDir $installerIni $archiveData

    Write-Host "###############################"
}



# Create temporary directory
function NewTempDir {
    $parent = [System.IO.Path]::GetTempPath()
    [string] $name = [System.Guid]::NewGuid()
    New-Item -ItemType Directory -Path (Join-Path $parent $name)
}



# Main function
function Main() {

    $packageName = $env:ChocolateyPackageName
    $url = 'https://download.microsoft.com/download/word97win/Utility/4.03/WIN98/EN-US/Hcwsetup.exe'
    $checksum = 'F666E3C1A12C750167F407C48D4371065C911942CAA42847E710B7584D23F81B'

    $tmpDir = $null
    try {
        # Create temp dir for extraction / processing
        $tmpDir = NewTempDir
        Write-Host "Temp dir: $tmpDir"

        # Download setup
        $hcSetupExe = Join-Path $tmpDir "Hcwsetup.exe"
        Get-ChocolateyWebFile -PackageName $packageName -FileFullPath $hcSetupExe -Url  $url -Checksum $checksum -ChecksumType "sha256" | Out-Null

        # Always install to "C:\Program Files" like original setup
        $pf = $env:ProgramW6432
        if ($null -eq $pf) {
            $pf = $env:ProgramFiles
        }
        $installDir = Join-Path $pf $INSTALL_FOLDER

        Write-Host "HelpWorkshop Setup downloaded to $hcSetupExe"

        DoInstall $hcSetupExe $installDir $tmpDir

    }
    finally {
        # Cleanup
        if (Test-Path $tmpDir) {
            Write-Host "Removing temp dir $tmpDir"
            Remove-Item -Recurse -Force $tmpDir
        }
    }
}

Main
