#pragma once

// Log install actions
class CInstallLog
{
private:
    // Log file pointer
    FILE* m_fpLogFile;

    // Log file name
    std::wstring m_logFileName;

    // Write string to file
    void WriteToFile(const std::wstring line) const;

public:
    // Constructor
    CInstallLog(const std::wstring logFileName);

    // Disallow copy
    CInstallLog(const CInstallLog&) = delete;

    // Destructor
    ~CInstallLog();

    // Log creation of file
    void LogCreatedFile(const std::wstring fileName) const;

    // Log creation of registry key
    void LogCreatedRegKey(const std::wstring regKeyPath) const;

    // Log creation of directory
    void LogCreatedDir(const std::wstring dirName) const;
};