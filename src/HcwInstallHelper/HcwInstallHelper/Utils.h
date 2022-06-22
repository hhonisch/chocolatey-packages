#pragma once

///////////////////////////////
// Helper macros

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define UNIQUE_NAME(base) CONCAT(base, __COUNTER__)

///////////////////////////////
// Error check macros

#define exit_error(i) throw i

// Fail if expression is false, output Win32 error message based on GetLastError
#define CHECK_FAIL_WIN32(expr) (void)(                     \
            (!!(expr)) ||                                  \
            (OutputWin32Error(GetLastError()), exit_error(1), 0) \
        )

// Fail if no memory has been allocated
#define CHECK_MEM_ALLOC(ptr) (void)(                       \
            (!!(ptr)) ||                                  \
            (OutputWin32Error(E_OUTOFMEMORY), exit_error(1), 0)  \
        )

// Fail if expression is false, output Win32 error message based on given error code
#define CHECK_FAIL_WIN32_ERROR(expr, error) (void)(         \
            (!!(expr)) ||                                   \
            (OutputWin32Error(error), exit_error(1), 0)           \
        )

// Fail if expression is false, output message
#define CHECK_FAIL_MSG(expr, message, ...) (void)(                     \
            (!!(expr)) ||                                              \
            (OutputErrorMessage(L"" message, __VA_ARGS__), exit_error(1), 0) \
        )

///////////////////////////////
// String macros

// Fix newlines to CRLF
#define STRING_FIX_NEWLINE_CRLF(str) STRING_FIX_NEWLINE_CRLF_INTERNAL(str, UNIQUE_NAME(ptr))
#define STRING_FIX_NEWLINE_CRLF_INTERNAL(str, ptrName)                      \
    std::unique_ptr<WCHAR[]> ptrName(0);                                    \
    if (str != NULL)                                                        \
    {                                                                       \
        size_t lenFixed = StringFixNewlineCrLf(str, NULL, NULL);            \
        if (lenFixed > 0)                                                   \
        {                                                                   \
            lenFixed++;                                                     \
            ptrName.reset(new WCHAR[lenFixed]);                             \
            if (ptrName.get() != NULL)                                      \
            {                                                               \
                StringFixNewlineCrLf(str, (LPWSTR)ptrName.get(), lenFixed); \
                str = ptrName.get();                                        \
            }                                                               \
        }                                                                   \
    }

// Make sure string is terminated with CRLF
#define STRING_TERMINATE_CRLF(str) STRING_TERMINATE_CRLF_INTERNAL(str, UNIQUE_NAME(ptr))
#define STRING_TERMINATE_CRLF_INTERNAL(str, ptrName)                        \
    std::unique_ptr<WCHAR[]> ptrName(0);                                    \
    if (str != NULL)                                                        \
    {                                                                       \
        size_t len = wcslen(str);                                           \
        if ((len < 2) || (str[len - 1] != '\n') || (str[len - 2] != '\r'))  \
        {                                                                   \
            len += 3;                                                       \
            ptrName.reset(new WCHAR[len]);                                  \
            if (ptrName.get() != NULL)                                      \
            {                                                               \
                wcscpy_s(ptrName.get(), len, str);                          \
                ptrName[len - 3] = '\r';                                    \
                ptrName[len - 2] = '\n';                                    \
                ptrName[len - 1] = '\0';                                    \
                str = ptrName.get();                                        \
            }                                                               \
        }                                                                   \
    }

///////////////////////////////
// RAII templates

// Resource wrapper for FILE
typedef std::unique_ptr<FILE, decltype(&fclose)> ResourcceWrapperFILE;
ResourcceWrapperFILE MakeResourceWrapperFILE(FILE& file);

// Resource wrapper for HANDLE
struct ResourceWrapperHANDLE_Deleter
{
    void operator()(HANDLE* handle)
    {
        CloseHandle(*handle);
    }
};
typedef std::unique_ptr<HANDLE, ResourceWrapperHANDLE_Deleter> ResourcceWrapperHANDLE;
ResourcceWrapperHANDLE MakeResourceWrapperHANDLE(HANDLE& handle);

// Resource wrapper for HKEY
struct ResourceWrapperHKEY_Deleter
{
    void operator()(HKEY* hkey)
    {
        RegCloseKey(*hkey);
    }
};
typedef std::unique_ptr<HKEY, ResourceWrapperHKEY_Deleter> ResourcceWrapperHKEY;
ResourcceWrapperHKEY MakeResourceWrapperHKEY(HKEY& hkey);

///////////////////////////////
// Function headers

// Check if stdout is redirected
bool IsStdOutRedirected();

// Check if stderr is redirected
bool IsStdErrRedirected();

// Check if console handle is redirected
bool IsConsoleHandleRedirected(HANDLE hConsole);

// Trim string left
std::wstring TrimStringLeft(const std::wstring s);

// Trim string right
std::wstring TrimStringRight(const std::wstring s);

// Trim string
std::wstring TrimString(const std::wstring s);

// Format string
std::wstring FormatString(LPCWSTR fmt, ...);

// Split string
std::vector<std::wstring> SplitString(const std::wstring str, const wchar_t separator);

// Check whether string starts with given prefix
bool StringStartsWith(const std::wstring str, const std::wstring prefix);

// Check whether string ends with given suffix
bool StringEndsWith(const std::wstring str, const std::wstring suffix);

// Convert wide string to string using codepage
BOOL WideStringToString(const std::wstring wideStr, std::string& str, UINT codepage = CP_UTF8);

// Convert string to wide string using codepage
BOOL StringToWideString(const std::string str, std::wstring& wideStr, UINT codepage = CP_UTF8);

// Write output
void WriteOutput(LPCWSTR str);

// Write text to output formatted
void WriteOutputFormatted(LPCWSTR fmt, ...);

// Set last Win32 error from HRESULT
void SetLastWin32ErrorFromHRESULT(HRESULT hr);

// Get Win32 error message from last error
std::wstring GetLastWin32ErrorMessage();

// Get Win32 error message
std::wstring GetWin32ErrorMessage(DWORD dwError);

// Get CRT error message
std::wstring GetCrtErrorMessage(errno_t errnum);

// Output error message for given Win32 error code
void OutputWin32Error(DWORD dwError);

// Output error message
void OutputErrorMessage(LPCWSTR fmt, ...);

// Create directory including intermediate dirs
bool CreateDirectoryDeep(LPCWSTR path);

// Create directory including intermediate dirs, fail on error
void CreateDirectoryDeep_FailOnError(LPCWSTR path);

// Get full path for file
bool GetFullPath(const std::wstring path, std::wstring& fullPath);

// Get full path for file, fail on error
void GetFullPath_FailOnError(const std::wstring path, std::wstring& fullPath);

// Combine paths
bool CombinePaths(const std::wstring parent, const std::wstring child, std::wstring& combinedPath);

// Combine paths, fail on error
void CombinePaths_FailOnError(const std::wstring parent, const std::wstring child, std::wstring& combinedPath);

// Set file attributes, fail on error
void SetFileAttributes_FailOnError(LPCWSTR lpFileName, DWORD dwFileAttributes);

// Copy file, fail on error
void CopyFile_FailOnError(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);

// Convert time_t to Windows FILETIME
void TimetToFileTime(time_t t, LPFILETIME pft);

// Get known folder path
BOOL GetKnownFolderPath(int csidl, std::wstring& folderPath);

// Get known folder path, fail on error
void GetKnownFolderPath_FailOnError(int csidl, std::wstring& folderPath);

// Check whether directory exists
bool DirExists(LPCWSTR dirName);
