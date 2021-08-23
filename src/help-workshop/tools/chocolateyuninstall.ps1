$ErrorActionPreference = 'Stop'

#################
# C# code
#################
[string] $code = @'
using System;
using System.Runtime.InteropServices;
using System.Text;

public class IniFile
{
    // Path to ini file
    private readonly string path;

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
if (-not ([System.Management.Automation.PSTypeName]'IniFile').Type) {
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

# Uninstall registry path 32 bit and 64 bit
[string] $REG_PATH_UNINSTALL_32 = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Help Workshop'
[string] $REG_PATH_UNINSTALL_64 = 'HKLM:\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Help Workshop'

# Name of start menu group
[string] $START_MENU_GROUP = "Microsoft Help Workshop"

# Name of install logfile
[string] $INSTALL_LOGFILE = "Help Workshop.log"



#####################
# Functions
#####################

# Remove files
function RemoveFiles([IniFile] $setupLog) {
    Write-Host "Removing files..."

    # Get install dir
    $v = $setupLog.GetString("destinations", "0", "")
    [string] $installDir = ($v -Split '\|')[2]
    Write-Host "  Install dir: $installDir"

    if (-not (Test-Path $installDir)) {
        Write-Host "WARNING: Install dir not found"
        return
    }

    # Read filenames from sections and remove files
    foreach ($sectionName in @("install"; "hcw"; "dbhe"; "Graphics" )) {
        [string[]] $sectionLines = $setupLog.GetSection($sectionName)
        foreach ($line in $sectionLines) {
            [string] $fileName = ($line -Split '=')[0]
            [string] $fullFileName = Join-Path $installDir $fileName
            if (Test-Path $fullFileName) {
                Write-Host "  Removing $fileName..."
                Remove-Item -Force $fullFileName | Out-Null
            }
        }
    }

    # Try to remove folder, will only succeed if empty
    try {
        Write-Host "  Removing $installDir..."
        [System.IO.Directory]::Delete($installDir)
    }
    catch {
        Write-Host "WARNING: Failed to remove $installDir"
    }

}


# Remove start menu entries
function RemoveStartMenuEntries([IniFile] $setupLog) {
    Write-Host "Removing start menu entries..."

    # Get folder
    [string] $startMenuFolder = Join-Path ([HelperUtils]::GetSpecialFolderPath(0x17)) $START_MENU_GROUP
    Write-Host "  Start menu folder: $startMenuFolder"

    if (-not (Test-Path $startMenuFolder)) {
        Write-Host "WARNING: Start menu folder not found"
        return
    }


    # Removing entries
    [string[]] $sectionLines = $setupLog.GetSection("targets")
    foreach ($line in $sectionLines) {
        $v = $line -Split '='
        $shortcutParams = ($v[1]) -Split '\|'
        [string] $fileName = $shortcutParams[3] + ".lnk"
        [string] $fullFileName = Join-Path $startMenuFolder $fileName
        if (Test-Path $fullFileName) {
            Write-Host "  Removing $fileName..."
            Remove-Item -Force $fullFileName | Out-Null
        }
    }

    # Try to remove folder, will only succeed if empty
    try {
        Write-Host "  Removing $startMenuFolder..."
        [System.IO.Directory]::Delete($startMenuFolder)
    }
    catch {
        Write-Host "WARNING: Failed to remove $startMenuFolder"
    }

}



# Remove registry keys
function RemoveRegKeys([string] $installDir) {

    # Removing registry keys
    Write-Host "Removing uninstall registry key..."
    if ($env:PROCESSOR_ARCHITECTURE -eq "AMD64") {
        [string] $regPath = $REG_PATH_UNINSTALL_64
    }
    else {
        [string] $regPath = $REG_PATH_UNINSTALL_32
    }
    if (-not (Test-Path $regPath)) {
        Write-Host "WARNING: Registry key not found"
        return
    }

    Remove-Item -Path $regPath -Force | Out-Null
}



# Main function
function Main() {

    # Load install logfile
    Write-Host "Uninstalling Help Workshop..."
    [string] $setupLogFilename = Join-Path $env:windir $INSTALL_LOGFILE
    if (-not (Test-Path $setupLogFilename)) {
        Write-Host "WARNING: Setup log not found: $setupLogFilename"
        return
    }

    [IniFile] $setupLog = New-Object IniFile($setupLogFilename)

    # Remove files
    RemoveFiles $setupLog

    # Remove start menu entries
    RemoveStartMenuEntries $setupLog

    # Remove registry entries
    RemoveRegKeys

    # Remove ini file
    Write-Host "Removing $setupLogFilename..."
    Remove-Item $setupLogFilename -Force | Out-Null

    Write-Host "Done"
}

Main
