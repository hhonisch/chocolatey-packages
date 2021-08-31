$ErrorActionPreference = 'Stop'

$packageName = $env:ChocolateyPackageName
$hcwSetupDownloadUrl = 'https://download.microsoft.com/download/word97win/Utility/4.03/WIN98/EN-US/Hcwsetup.exe'
$hcwSetupChecksum = 'F666E3C1A12C750167F407C48D4371065C911942CAA42847E710B7584D23F81B'

[string] $toolsDir = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition

# Parse package params
$pp = Get-PackageParameters
# Install dir
if (!$pp['InstallDir']) { 
    # Set default install dir to 32 bit Program Files folder
    if (${env:ProgramFiles(x86)}) { $pp['InstallDir'] = Join-Path ${env:ProgramFiles(x86)} "Help Workshop" }
    else { $pp['InstallDir'] = Join-Path $env:ProgramFiles "Help Workshop" }
}
# Location of help workshop setup
if (!$pp['HcwSetupLocation']) { 
    $pp['HcwSetupLocation'] = $hcwSetupDownloadUrl
}

$tmpDir = $null
try {
    # Create temp dir for extraction / processing
    $tmpDir = Join-Path ([System.IO.Path]::GetTempPath()) ([System.Guid]::NewGuid())
    Write-Host "Create temp dir $tmpDir"
    New-Item -ItemType Directory -Path $tmpDir | Out-Null

    # Download setup
    $hcSetupExe = Join-Path $tmpDir "Hcwsetup.exe"
    Get-ChocolateyWebFile -PackageName $packageName -FileFullPath $hcSetupExe -Url  ($pp['HcwSetupLocation']) -Checksum $hcwSetupChecksum -ChecksumType "sha256" | Out-Null
    Write-Host "HelpWorkshop Setup downloaded to $hcSetupExe"

    Write-Host "Extracting $hcSetupExe"
    Get-ChocolateyUnzip -FileFullPath $hcSetupExe -Destination $tmpDir | Out-Null

    Write-Host "Running HcwInstallHelper..."
    $args = "install `"$tmpDir`" `"$($pp['InstallDir'])`""
    Start-ChocolateyProcessAsAdmin $args "$toolsDir\HcwInstallHelper.exe"
}
finally {
    # Cleanup temp dir
    if (Test-Path $tmpDir) {
        Write-Host "Removing temp dir $tmpDir"
        Remove-Item -Recurse -Force $tmpDir
    }
}