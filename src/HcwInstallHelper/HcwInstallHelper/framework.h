#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _WIN32_WINNT _WIN32_WINNT_WIN7  // Target windows 7 or higher
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Debug memory allocs in debug build
#ifdef _DEBUG
#define MYDEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
#define new MYDEBUG_NEW
#define _CRTDBG_MAP_ALLOC
#endif

// Begin: Disable some warnings in external code
#pragma warning(push)
#pragma warning(disable: 26451)
#pragma warning(disable: 26495)
#pragma warning(disable: 26812)
#pragma warning(disable: 6001)

//////////////////////////
// Windows
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <shlobj.h>

//////////////////////////
// STL
#include <string>
#include <vector>
#include <memory>
#include <functional>

//////////////////////////
// ZLIB
#include <zlib.h>

// End: Disable some warnings in external code
#pragma warning(pop)

//////////////////////////
// Some constants

// App path registry path
#define REG_PATH_APP_PATHS "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths"

// Name of start menu group
#define START_MENU_GROUP "Microsoft Help Workshop"

// Name of install logfile
#define INSTALL_LOGFILE "hwinstall.log"

// Name of installer ini
#define INSTALLER_INI_NAME "_instpgm.ini"
