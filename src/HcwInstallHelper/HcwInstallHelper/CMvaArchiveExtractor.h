#pragma once

// Forward declarations
struct MvaArchiveItemHeader;

// Callback function templates
typedef std::function<void(const std::wstring&, const std::wstring&)> FnCallbackBeforeFileExtract;
typedef std::function<void(const std::wstring&, const std::wstring&)> FnCallbackFileExtracted;

// Extract Mva archive
class CMvaArchiveExtractor
{
private:
    // Callback before a file is extracted
    FnCallbackBeforeFileExtract m_fnCallbackBeforeFileExtract;

    // Callback after a file is extracted
    FnCallbackFileExtracted m_fnCallbackFileExtracted;

    // Extract archive item
    void ExtractArchiveItem(FILE* archive, MvaArchiveItemHeader& itemHeader, const std::wstring destFileName);

    // Compute CRC32-like MVA checksum
    static unsigned int ComputeCrc32MvaChecksum(unsigned char* data, size_t length);

public:
    // Extract archive
    void ExtractArchive(const std::wstring mvaFileName, const std::wstring destDir);

    // Set before file extracted callback
    void SetCallbackBeforeFileExtract(FnCallbackBeforeFileExtract fnCallbackBeforeFileExtract);

    // Set after file extracted callback
    void SetCallbackFileExtracted(FnCallbackFileExtracted fnCallbackFileExtracted);
};
