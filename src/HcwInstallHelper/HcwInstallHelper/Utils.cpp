#include "pch.h"
#include "Utils.h"

// Make FILE resource wrapper
ResourcceWrapperFILE MakeResourceWrapperFILE(FILE& file)
{
    ResourcceWrapperFILE wrapper(&file, &fclose);
    return wrapper;
}

// Make HANDLE resource wrapper
ResourcceWrapperHANDLE MakeResourceWrapperHANDLE(HANDLE& handle)
{
    ResourcceWrapperHANDLE wrapper(&handle, ResourceWrapperHANDLE_Deleter{});
    return wrapper;
}

// Make HKEY resource wrapper
ResourcceWrapperHKEY MakeResourceWrapperHKEY(HKEY& hkey)
{
    ResourcceWrapperHKEY wrapper(&hkey, ResourceWrapperHKEY_Deleter{});
    return wrapper;
}

// Check if stdout is redirected
bool IsStdOutRedirected()
{
    static bool isCached = false;
    static bool isRedirected;
    if (!isCached)
    {
        isRedirected = IsConsoleHandleRedirected(GetStdHandle(STD_OUTPUT_HANDLE));
        isCached = true;
    }
    return isRedirected;
}

// Check if stderr is redirected
bool IsStdErrRedirected()
{
    static bool isCached = false;
    static bool isRedirected;
    if (!isCached)
    {
        isRedirected = IsConsoleHandleRedirected(GetStdHandle(STD_ERROR_HANDLE));
        isCached = true;
    }
    return isRedirected;
}

// Check if console handle is redirected
bool IsConsoleHandleRedirected(HANDLE hConsole)
{
    if (hConsole != INVALID_HANDLE_VALUE)
    {
        DWORD dwFiletype = GetFileType(hConsole);
        if (!((dwFiletype == FILE_TYPE_UNKNOWN) && (GetLastError() != ERROR_SUCCESS)))
        {
            DWORD mode;
            dwFiletype &= ~(FILE_TYPE_REMOTE);
            if (dwFiletype == FILE_TYPE_CHAR)
            {
                bool retval = GetConsoleMode(hConsole, &mode);
                if ((retval == false) && (GetLastError() == ERROR_INVALID_HANDLE))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
    }
    // TODO: Not even a stdout so this is not even a console?
    return false;
}

// Whitespace for trimming
const std::wstring WHITESPACE = L" \n\r\t\f\v";

// Trim string left
std::wstring TrimStringLeft(const std::wstring s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::wstring::npos) ? L"" : s.substr(start);
}

// Trim string right
std::wstring TrimStringRight(const std::wstring s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::wstring::npos) ? L"" : s.substr(0, end + 1);
}

// Trim string right
std::wstring TrimString(const std::wstring s)
{
    return TrimStringRight(TrimStringLeft(s));
}

// Format string
std::wstring FormatString(LPCWSTR fmt, ...)
{
    CHECK_FAIL_WIN32_ERROR(fmt != NULL, ERROR_INVALID_PARAMETER);

    LPWSTR messageBuffer = NULL;
    va_list ap;
    va_start(ap, fmt);

    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                                 fmt, 0, 0, (LPWSTR)&messageBuffer, 0, &ap);
    va_end(ap);

    if (size != 0)
    {
        // Smart pointer for auto-free
        std::unique_ptr<WCHAR[], decltype(&LocalFree)> ptrMessageBuffer(messageBuffer, &LocalFree);

        // Write output
        return messageBuffer;
    }
    else
    {
        return L"";
    }
}

// Split string
std::vector<std::wstring> SplitString(const std::wstring str, const wchar_t separator)
{
    std::vector<std::wstring> result;
    std::wstring::const_iterator itLastPos = str.begin();
    std::wstring::const_iterator itPos;
    for (itPos = str.begin(); itPos != str.end(); itPos++)
    {
        if (*itPos == separator)
        {
            std::wstring token(itLastPos, itPos);
            result.push_back(token);
            itLastPos = itPos + 1;
        }
    }
    std::wstring token(itLastPos, itPos);
    result.push_back(token);
    return result;
}

// Check whether string starts with given prefix
bool StringStartsWith(const std::wstring str, const std::wstring prefix)
{
    if (prefix.size() > str.size())
    {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

// Check whether string starts with given suffix
bool StringEndsWith(const std::wstring str, const std::wstring suffix)
{
    if (suffix.size() > str.size())
    {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

// Convert wide string to string using codepage
BOOL WideStringToString(const std::wstring wideStr, std::string& str, UINT codepage)
{
    // Start with an initial buffer size of 4 times the original string length, which should be enough 99% of the time
    DWORD bufferLen = wideStr.size() * 4;

    // Loop until success
    do
    {
        str.resize(bufferLen);
        int bytesWritten = WideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, wideStr.c_str(), wideStr.size(), &str[0], bufferLen, NULL, NULL);

        // Success
        if (bytesWritten > 0)
        {
            str.resize(bytesWritten);
            return TRUE;
        }

        // Handle buffer too small
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            bufferLen *= 2;
            continue;
        }

        // else fail
        return FALSE;
    } while (true);
}

// Convert string to wide string using codepage
BOOL StringToWideString(const std::string str, std::wstring& wideStr, UINT codepage)
{
    // Start with an initial buffer size the same size as the original string length, which should be enough 99% of the time
    DWORD bufferLen = str.size() + 1;

    // Loop until success
    do
    {
        wideStr.resize(bufferLen);
        int bytesWritten = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, str.c_str(), str.size(), &wideStr[0], bufferLen);

        // Success
        if (bytesWritten > 0)
        {
            wideStr.resize(bytesWritten);
            return TRUE;
        }

        // Handle buffer too small
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            bufferLen *= 2;
            continue;
        }

        // else fail
        return FALSE;
    } while (true);
}

// Convert string newlines to crlf
size_t StringFixNewlineCrLf(LPCWSTR strSrc, LPWSTR strDest, DWORD cchDestLen)
{
    const wchar_t* pSrc = strSrc;
    wchar_t* pDest = strDest;
    int state = 0;
    size_t cchDestLenRequired = 0;
    while (*pSrc != L'\0')
    {
        switch (state)
        {
        case 0:
            // Standard state
            if (*pSrc == L'\r')
            {
                state = 1;
                break;
            }
            else if (*pSrc == L'\n')
            {
                // Insert \r before \n
                if (pDest && (DWORD(pDest - strDest) < cchDestLen))
                {
                    *pDest = '\r';
                    pDest++;
                }
                cchDestLenRequired++;
            }
            break;

        case 1:
            // Last char was CR
            if (*pSrc == L'\r')
            {
                state = 1;
                break;
            }
            else
            {
                state = 0;
            }
            break;
        }
        // Copy to dest buffer only if valid pointer
        if (pDest && (DWORD(pDest - strDest) < cchDestLen))
        {
            *pDest = *pSrc;
            pDest++;
        }
        cchDestLenRequired++;
        pSrc++;
    }
    // Add terminating \0
    if (pDest && (DWORD(pDest - strDest) < cchDestLen))
    {
        *pDest = L'\0';
    }
    return cchDestLenRequired;
}

// Write text to output raw / no CRLF string conversions
void WriteOutputRaw(LPCWSTR str)
{
    // Convert newline to CRLF
    if (IsStdOutRedirected())
    {
        // If output redirected to file, use console CP
        UINT cp = GetConsoleOutputCP();
        std::string encodedStr;
        CHECK_FAIL_WIN32(WideStringToString(str, encodedStr, cp));
        DWORD dwBytesWritten;
        CHECK_FAIL_WIN32(WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), encodedStr.c_str(), encodedStr.size(), &dwBytesWritten, NULL));
    }
    else
    {
        // If output goes to console, write unicode
        size_t strLen = wcslen(str);
        CHECK_FAIL_WIN32(WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), str, strLen, NULL, NULL));
    }
}

// Write text to output
void WriteOutput(LPCWSTR str)
{
    CHECK_FAIL_WIN32_ERROR(str != NULL, ERROR_INVALID_PARAMETER);
    STRING_FIX_NEWLINE_CRLF(str);
    WriteOutputRaw(str);
}

// Write text to output formatted
void WriteOutputFormatted(LPCWSTR fmt, ...)
{
    CHECK_FAIL_WIN32_ERROR(fmt != NULL, ERROR_INVALID_PARAMETER);

    LPWSTR messageBuffer = NULL;
    va_list ap;
    va_start(ap, fmt);

    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                                 fmt, 0, 0, (LPWSTR)&messageBuffer, 0, &ap);
    va_end(ap);

    if (size != 0)
    {
        // Smart pointer for auto-free
        std::unique_ptr<WCHAR[], decltype(&LocalFree)> ptrMessageBuffer(messageBuffer, &LocalFree);

        // Write output
        WriteOutputRaw(messageBuffer);
    }
    else
    {
        WriteOutputRaw(L"(Error formatting message):");
        WriteOutputRaw(fmt);
    }
}

// Write error raw / no CRLF string conversions
void WriteErrorRaw(LPCWSTR str)
{
    // Convert newline to CRLF
    if (IsStdOutRedirected())
    {
        // If output redirected to file, use console CP
        UINT cp = GetConsoleOutputCP();
        std::string encodedStr;
        if (WideStringToString(str, encodedStr, cp))
        {
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), encodedStr.c_str(), encodedStr.size(), NULL, NULL);
        }
        else
        {
            // If converting to codepage fails, write unicode
            size_t strLen = wcslen(str);
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), str, strLen * sizeof(wchar_t), NULL, NULL);
        }
    }
    else
    {
        // If output goes to console, write unicode
        size_t strLen = wcslen(str);
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), str, strLen, NULL, NULL);
    }
}

// Set last Win32 error from HRESULT
void SetLastWin32ErrorFromHRESULT(HRESULT hr)
{
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
    {
        SetLastError(HRESULT_CODE(hr));
    }
    else
    {
        SetLastError(ERROR_GEN_FAILURE);
    }
}

// Get Win32 error message from last error
std::wstring GetLastWin32ErrorMessage()
{
    return GetWin32ErrorMessage(GetLastError());
}

// Get Win32 error message
std::wstring GetWin32ErrorMessage(DWORD dwError)
{
    LPWSTR messageBuffer = NULL;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

    if (size > 0)
    {
        // Auto-free messageBuffer
        std::unique_ptr<WCHAR[], decltype(&LocalFree)> ptrMessageBuffer(messageBuffer, &LocalFree);

        // Trim trailing newline
        if ((size > 2) && (messageBuffer[size - 1] == '\n') && (messageBuffer[size - 2] == '\r'))
        {
            messageBuffer[size - 2] = '\0';
        }
        return messageBuffer;
    }
    else
    {
        return L"";
    }
}

// Get CRT error message
std::wstring GetCrtErrorMessage(errno_t errnum)
{
    wchar_t buf[2048];
    if (_wcserror_s(buf, _countof(buf), errnum) == 0)
    {
        return buf;
    }
    else
    {
        return L"(unable to determine error message)";
    }
}

// Output error message for given Win32 error code
void OutputWin32Error(DWORD dwError)
{
    LPWSTR messageBuffer = NULL;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
    if (size != 0)
    {
        std::unique_ptr<WCHAR[], decltype(&LocalFree)> ptrMessageBuffer(messageBuffer, &LocalFree);
        STRING_TERMINATE_CRLF(messageBuffer);
        WriteErrorRaw(messageBuffer);
    }
    else
    {
        WriteOutputRaw(L"(Error retrieving message)");
    }
}

// Output error message
void OutputErrorMessage(LPCWSTR fmt, ...)
{
    LPWSTR messageBuffer = NULL;
    va_list ap;
    va_start(ap, fmt);

    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                                 fmt, 0, 0, (LPWSTR)&messageBuffer, 0, &ap);
    va_end(ap);
    // Save original value of allocated buffer
    LPWSTR messageBufferOrg = messageBuffer;
    if (size != 0)
    {
        size_t len = wcslen(messageBuffer);
        STRING_TERMINATE_CRLF(messageBuffer);
        WriteErrorRaw(messageBuffer);

        // Free saved pointer, as macros above might have changed it
        LocalFree(messageBufferOrg);
    }
    else
    {
        STRING_FIX_NEWLINE_CRLF(fmt);
        STRING_TERMINATE_CRLF(fmt);
        WriteErrorRaw(fmt);
    }
}

// Find last path segment
LPCWSTR FindLastPathSegment(LPCWSTR path)
{
    size_t pathLength = wcslen(path);
    // If there is a trailing slash ignore that in the search.
    size_t limitedLength = ((pathLength > 0) && (path[pathLength - 1] == L'\\')) ? (pathLength - 1) : pathLength;

    LPCWSTR result;
    int const offset = FindStringOrdinal(FIND_FROMEND, path, static_cast<int>(limitedLength), L"\\", 1, TRUE);
    if (offset == -1)
    {
        result = path + pathLength; // null terminator
    }
    else
    {
        result = path + offset + 1; // just past the slash
    }
    return result;
}

// Get range of parent path
bool TryGetParentPathRange(PCWSTR path, _Out_ size_t* parentPathLength)
{
    *parentPathLength = 0;
    bool hasParent = false;
    LPCWSTR rootEnd = PathSkipRoot(path);
    if ((rootEnd != NULL) && (*rootEnd != L'\0'))
    {
        LPCWSTR lastSegment = FindLastPathSegment(path);
        if (*lastSegment == '\0')
        {
            return false;
        }
        *parentPathLength = lastSegment - path;
        hasParent = (*parentPathLength != 0);
    }
    return hasParent;
}

// Create directory including intermediate dirs
bool CreateDirectoryDeep(LPCWSTR path)
{
    if (CreateDirectoryW(path, NULL) == FALSE)
    {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_PATH_NOT_FOUND)
        {
            size_t parentLength;
            if (TryGetParentPathRange(path, &parentLength))
            {
                std::unique_ptr<wchar_t[]> parent(new wchar_t[parentLength + 1]);
                CHECK_MEM_ALLOC(parent.get());
                if (!SUCCEEDED(StringCchCopyNW(parent.get(), parentLength + 1, path, parentLength)))
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    return false;
                }
                if (!CreateDirectoryDeep(parent.get()))
                {
                    return false;
                }
            }
            if (CreateDirectoryW(path, NULL) == FALSE)
            {
                lastError = GetLastError();
                if (lastError != ERROR_ALREADY_EXISTS)
                {
                    return false;
                }
            }
        }
        else if (lastError != ERROR_ALREADY_EXISTS)
        {
            return false;
        }
    }
    return true;
}

// Create directory including intermediate dirs, fail on error
void CreateDirectoryDeep_FailOnError(LPCWSTR path)
{
    CHECK_FAIL_MSG(CreateDirectoryDeep(path), "Error creating directory \"%1\": %2", path, GetLastWin32ErrorMessage().c_str());
}

// Get full path for file
bool GetFullPath(const std::wstring path, std::wstring& fullPath)
{
    size_t bufLen = MAX_PATH;
    fullPath.resize(bufLen);
    DWORD dwLen = GetFullPathName(path.c_str(), bufLen, &fullPath[0], NULL);
    if (dwLen > bufLen)
    {
        bufLen = dwLen;
        fullPath.resize(bufLen);
        dwLen = GetFullPathName(path.c_str(), bufLen, &fullPath[0], NULL);
    }
    fullPath.resize(dwLen);
    return (dwLen > 0);
}

// Get full path for file, fail on error
void GetFullPath_FailOnError(const std::wstring path, std::wstring& fullPath)
{
    CHECK_FAIL_MSG(GetFullPath(path, fullPath), "Error determining full path for\"%1\": %2", path.c_str(), GetLastWin32ErrorMessage().c_str());
}

// Combine paths
bool CombinePaths(const std::wstring parent, const std::wstring child, std::wstring& combinedPath)
{
    size_t bufLen = MAX_PATH;
    combinedPath.resize(bufLen);
    if (!PathCombine(&combinedPath[0], parent.c_str(), child.c_str()))
    {
        SetLastError(E_INVALIDARG);
        return false;
    }
    size_t p = combinedPath.find(L'\0');
    combinedPath.resize(p);
    return true;
}

// Combine paths, fail on error
void CombinePaths_FailOnError(const std::wstring parent, const std::wstring child, std::wstring& combinedPath)
{
    CHECK_FAIL_MSG(CombinePaths(parent, child, combinedPath), "Error combining paths \"%1\" and \"%2\": %3",
        parent.c_str(), child.c_str(), GetLastWin32ErrorMessage().c_str());
}

// Set file attributes, fail on error
void SetFileAttributes_FailOnError(LPCWSTR lpFileName, DWORD dwFileAttributes)
{
    CHECK_FAIL_MSG(SetFileAttributes(lpFileName, dwFileAttributes), "Error setting file attributes for \"%1\": %2", lpFileName,
        GetLastWin32ErrorMessage().c_str());
}

// Copy file, fail on error
void CopyFile_FailOnError(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
    CHECK_FAIL_MSG(CopyFile(lpExistingFileName, lpNewFileName, bFailIfExists), "Error copying file \"%1\" to \"%2\": %3",
        lpExistingFileName, lpNewFileName, GetLastWin32ErrorMessage().c_str());
}

// Convert time_t to Windows FILETIME
void TimetToFileTime(time_t t, LPFILETIME pft)
{
    ULARGE_INTEGER time_value;
    time_value.QuadPart = (t * 10000000LL) + 116444736000000000LL;
    pft->dwLowDateTime = time_value.LowPart;
    pft->dwHighDateTime = time_value.HighPart;
}

// Get known folder path
BOOL GetKnownFolderPath(int csidl, std::wstring& folderPath)
{
    folderPath.resize(MAX_PATH);
    HRESULT hr = SHGetFolderPath(0, csidl, 0, 0, &folderPath[0]);
    if (!SUCCEEDED(hr))
    {
        SetLastWin32ErrorFromHRESULT(hr);
        return FALSE;
    }
    size_t p = folderPath.find(L'\0');
    folderPath.resize(p);
    return TRUE;
}

// Get known folder path, fail on error
void GetKnownFolderPath_FailOnError(int csidl, std::wstring& folderPath)
{
    CHECK_FAIL_MSG(GetKnownFolderPath(csidl, folderPath), "Error retrieving known folder path");
}

// Check whether directory exists
bool DirExists(LPCWSTR dirName)
{
    DWORD ftyp = GetFileAttributes(dirName);
    if (ftyp == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    {
        return true;   // this is a directory!
    }
    else
    {
        return false;    // this is not a directory!
    }
}