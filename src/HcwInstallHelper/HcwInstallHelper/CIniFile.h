#pragma once
#include "pch.h"

// Ini file
class CIniFile
{
private:
    // Ini files to check
    std::wstring  m_iniFilePath;
public:
    // Constructor with given ini file path
    CIniFile(const std::wstring iniFilePath);

    // Set ini file path
    void SetIniFilePath(const std::wstring iniFilePath);

    // Get ini file path
    std::wstring GetIniFilePath() const;

    // Read string
    std::wstring ReadString(const std::wstring section, const std::wstring key, const std::wstring defaultValue) const;

    // Read integer
    int ReadInt(const std::wstring section, const std::wstring key, int defaultValue) const;
};
