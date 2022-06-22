// Headers
#include "pch.h"
#include "CIniFile.h"

// Buffer size for reading values single values
#define DATA_BUFFER_SINGLE_VALUE_SIZE 2048

// Constructor with given base name
CIniFile::CIniFile(const std::wstring baseName) : m_iniFilePath(baseName)
{}

// Set ini file path
void CIniFile::SetIniFilePath(const std::wstring iniFilePath)
{
    m_iniFilePath = iniFilePath;
}

// Get ini file path
std::wstring CIniFile::GetIniFilePath() const
{
    return m_iniFilePath;
}

// Read string
std::wstring CIniFile::ReadString(const std::wstring section, const std::wstring key, const std::wstring defaultValue) const
{
    std::wstring buffer;
    buffer.resize(DATA_BUFFER_SINGLE_VALUE_SIZE);
    DWORD dwResult = GetPrivateProfileStringW(section.c_str(), key.c_str(), defaultValue.c_str(), &buffer[0], DATA_BUFFER_SINGLE_VALUE_SIZE, m_iniFilePath.c_str());
    size_t p = buffer.find(L'\0');
    buffer.resize(p);
    return buffer;
}

// Read integer
int CIniFile::ReadInt(const std::wstring section, const std::wstring key, int defaultValue) const
{
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), defaultValue, m_iniFilePath.c_str());
}