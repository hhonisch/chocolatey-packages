#Requires -Version 5

# Terminate on exception
trap {
    Write-Host "Error: $_"
    Write-Host $_.ScriptStackTrace
    exit 1
}

# Always stop on errors
$ErrorActionPreference = "Stop"

# Strict mode
Set-StrictMode -Version Latest


# Set version for Chocolatey package
function SetVersionChocoPackage($ver) {
    Write-Host "Setting version in Chocolatey package"
    $nuspecPath = Join-Path $PSScriptRoot "..\src\package\help-workshop.nuspec"
    Write-Host "Reading $nuspecPath..."
    $nuspecOld = Get-Content $nuspecPath
    $nuspecNew = [System.Collections.ArrayList]::new()
    foreach ($line in $nuspecOld) {
        $line = $line -replace '^(\s*<version>).*(</version>.*)$' , "`${1}$($ver.str4)`${2}"
        $nuspecNew.Add($line) | Out-Null
    }
    Write-Host "Writing $nuspecPath..."
    $nuspecNew | Out-File "$nuspecPath" -Encoding "utf8"
    Write-Host "Done: Setting version in Chocolatey package"
}


# Set version for HcwInstallHelper
function SetVersionInstallHelper($ver) {
    Write-Host "Setting version in HcwInstallHelper"
    $rcPath = Join-Path $PSScriptRoot "..\src\HcwInstallHelper\HcwInstallHelper\Resource.rc"
    Write-Host "Reading $rcPath..."
    $rcOld = Get-Content $rcPath
    $rcNew = [System.Collections.ArrayList]::new()
    foreach ($line in $rcOld) {
        $line = $line -replace '^(\s*(?:FILE|PRODUCT)VERSION\s+).*$' , "`${1}$($ver.major),$($ver.minor),$($ver.patch),$($ver.build)"
        $line = $line -replace '^(\s*VALUE\s+"(?:File|Product)Version"\s*,\s*).*$' , "`${1}`"$($ver.major).$($ver.minor).$($ver.patch).$($ver.build)`""
        $rcNew.Add($line) | Out-Null
    }
    Write-Host "Writing $rcPath..."
    $rcNew | Out-File $rcPath -Encoding "unicode"
    Write-Host "Done: Setting version in HcwInstallHelper"
}


# Main function
function Main() {
    Write-Host "Setting version in source files"
    $verXmlPath = Join-Path $PSScriptRoot version.xml
    Write-Host "Reading version from $verXmlPath..."
    # Read version from XML file
    [xml] $verXml = Get-Content $verXmlPath
    $verStr = $verXml.version
    Write-Host "  => $verStr"
    $ver = @{"str" = $verStr }
    if ($verStr -match "(\d+)\.(\d+)\.(\d+)\.(\d+)") {
        $ver.major = $Matches[1]
        $ver.minor = $Matches[2]
        $ver.patch = $Matches[3]
        $ver.build = $Matches[4]
        $ver.str4 = "$($ver.major).$($ver.minor).$($ver.patch).$($ver.build)"
    }
    Write-Host "Setting to: $($ver.str4)"

    
    # Set version for chocolatey package
    SetVersionChocoPackage $ver

    # Set version for HcwInstallHelper
    SetVersionInstallHelper $ver

    Write-Host "Done: Setting version in source files"
}


Main