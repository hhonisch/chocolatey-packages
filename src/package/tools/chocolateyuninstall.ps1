$ErrorActionPreference = 'Stop'

[string] $toolsDir = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition

# find installation dir
$regPathHcw = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\hcw.exe'
$regPathHcrtf = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\hcrtf.exe'
$regPathDbhe = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\dbhe.exe'

$installDir = [System.IO.Path]::GetDirectoryName((Get-ItemProperty -Path @($regPathHcw, $regPathHcrtf, $regPathDbhe) -ErrorAction SilentlyContinue | Select-Object -ExpandProperty "(Default)" -First 1))

if (-not $installDir) {
    Write-Warning "$packageName has already been uninstalled by other means."
    return
}

Write-Host "Install dir: $installDir"

Write-Host "Running HcwInstallHelper..."
& "$toolsDir\HcwInstallHelper.exe" uninstall $installDir
if ($LASTEXITCODE -ne 0) {
    Write-Error "HcwInstallHelper returned an error"
}