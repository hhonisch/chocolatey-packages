#include "pch.h"
#include "CInstallLog.h"
#include "Utils.h"

// Constructor
CInstallLog::CInstallLog(const std::wstring logFileName)
    : m_logFileName(logFileName)
{
    // Open file ptr
    m_fpLogFile = _wfsopen(logFileName.c_str(), L"w, ccs=UTF-8", _SH_DENYWR);
    CHECK_FAIL_MSG(m_fpLogFile, "Error creating install log \"%1\": %2", logFileName.c_str(), GetCrtErrorMessage(errno).c_str());
}

// Destructor
CInstallLog::~CInstallLog()
{
    // Close file pointer
    if (m_fpLogFile != NULL)
    {
        fclose(m_fpLogFile);
    }
}

// Write string to file
void CInstallLog::WriteToFile(const std::wstring line) const
{
    CHECK_FAIL_MSG(fputws(line.c_str(), m_fpLogFile) >= 0, "Error writing to file %1\n", m_logFileName.c_str());
}

// Log creation of file
void CInstallLog::LogCreatedFile(const std::wstring fileName) const
{
    std::wstring tmp;
    tmp.reserve(fileName.size() + 100);
    tmp = L"CreateFile|";
    tmp += fileName;
    tmp += L"\n";
    WriteToFile(tmp);
}

// Log creation of registry key
void CInstallLog::LogCreatedRegKey(const std::wstring regKeyPath) const
{
    std::wstring tmp;
    tmp.reserve(regKeyPath.size() + 100);
    tmp = L"CreateRegKey|";
    tmp += regKeyPath;
    tmp += L"\n";
    WriteToFile(tmp);
}

// Log creation of directory
void CInstallLog::LogCreatedDir(const std::wstring dirName) const
{
    std::wstring tmp;
    tmp.reserve(dirName.size() + 100);
    tmp = L"CreateDir|";
    tmp += dirName;
    tmp += L"\n";
    WriteToFile(tmp);
}