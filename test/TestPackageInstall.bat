@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
set MYDIR=%~dp0
set MYDIR=%MYDIR:~0,-1%

set SCRIPT_TITLE=Test package install
echo ************************************************************
echo *** %SCRIPT_TITLE%: Start %DATE% %TIME:~0,-3%
echo ************************************************************

set PACKAGE_SOURCE_DIR=%MYDIR%\..\Dist
set PACKAGE_INSTALL_DIR=%MYDIR%\Help Workshop Install

echo.
echo *** Install chocolatey package ***
echo.
choco install help-workshop --source "%PACKAGE_SOURCE_DIR%" --yes --force --no-progress --params="'/InstallDir:%PACKAGE_INSTALL_DIR%'" || goto error

echo.
echo *** Test whether files exist in install dir
echo.
for %%I in (hcw.exe hcw.hlp hcrtf.exe dbhe.exe) do (
  echo Looking for %%I...
  if not exist "%PACKAGE_INSTALL_DIR%\%%I" (
    echo File not found: %PACKAGE_INSTALL_DIR%\%%I
    set ERROR_OCCURRED=1
  ) else (
    echo   ...found
  )
)

echo.
echo *** Uninstall chocolatey package ***
echo.
choco uninstall help-workshop --yes --force --no-progress" || goto error


goto end

:error
set ERROR_OCCURRED=1

:end
echo.
echo *** Get chocolatey.log
echo.
copy /Y "%ChocolateyInstall%\logs\chocolatey.log" "%MYDIR%\chocolatey.log"
echo.
echo ************************************************************
if not "%ERROR_OCCURRED%"=="" (
  echo *** %SCRIPT_TITLE%: ERROR^(s^) %DATE% %TIME:~0,-3%
  echo *** One or more errors occurred. Check the log.
) else (
  echo *** %SCRIPT_TITLE%: Finished %DATE% %TIME:~0,-3%
)
echo ************************************************************

if "%ERROR_OCCURRED%"=="1" (
  cmd /c exit 1
) else (
  cmd /c exit 0
)

