using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;

namespace HcwInstallHelper
{
    // Extract Mva archive
    public class MvaArchiveExtractor
    {
        // Valid Zlib headers
        private static ushort[] ZLIB_HEADERS = new ushort[] { 0x0178, 0x5e78, 0x9c78, 0xda78 };

        // Buffer for stream copy
        private static byte[] copyStreamBuffer;

        // Size for CopySteam buffer
        private const uint COPYSTREAM_BUFFER_SIZE = 0x8000;

        // Max path length
        private const ushort MAX_PATH = 260;

        // MVA archive header size
        private const byte MVA_ARCHIVE_HEADER_SIZE = 8;

        // MVA magic number archive header
        private const uint MVA_MAGICNUM_ARCHIVE = 0x686C666D;

        // MVA magic number archive item header
        private const uint MVA_MAGICNUM_ARCHIVE_ITEM = 0x6E65666D;

        // MVA magic number archive item header 2
        private const uint MVA_MAGICNUM_ARCHIVE_ITEM_2 = 0x7ffdf000;

        // MVA magic number archive item header 3
        private const uint MVA_MAGICNUM_ARCHIVE_ITEM_3 = 0x00000004;

        // Adler32 checksum size
        private const byte ADLER32_CHECKSUM_SIZE = 4;

        // Constructor
        public static void ExtractArchive(string mvaFileName, string destDir, List<string> extractedFiles, Action<string> onLogMessage)
        {
            onLogMessage($"Extracting archive {mvaFileName} to {destDir}");

            // Open archive as stream
            using (Stream fileStreamIn = new FileStream(mvaFileName, FileMode.Open, FileAccess.Read, FileShare.Read))
            using (Stream bufferedStreamIn = new BufferedStream(fileStreamIn))
            {
                var binReader = new System.IO.BinaryReader(bufferedStreamIn);

                // Read archive header
                ReadArchiveHeader(binReader);

                // Struct for item header data
                ArchiveItemHeader archiveItemHeader;
                while (binReader.PeekChar() != -1)
                {
                    // Read archive item header
                    ReadArchiveItemHeader(binReader, out archiveItemHeader);

                    // Build destination file name
                    string pureFileName = Path.GetFileName(archiveItemHeader.FileName);
                    string destFileName = Path.Combine(destDir, pureFileName);
                    extractedFiles.Add(pureFileName);

                    // Extract archive item
                    ExtractArchiveItem(binReader, archiveItemHeader, destFileName, onLogMessage);
                }
            }
        }


        // Read archive item header
        private static void ExtractArchiveItem(BinaryReader binReader, ArchiveItemHeader itemHeader, string destFileName, Action<string> onLogMessage)
        {
            onLogMessage($"  Extracting archive item {destFileName}...");

            // Save current stream position
            int streamPosStart = (int)binReader.BaseStream.Position;

            // Read and verify Zlib header
            var zlibHeader = binReader.ReadUInt16();
            if (!ZLIB_HEADERS.Contains(zlibHeader))
            {
                throw new Exception("Error: Invalid zlib header");
            }

            // Read compressed content
            int bytesToRead = (int)(itemHeader.SizeCompressed - binReader.BaseStream.Position + streamPosStart - ADLER32_CHECKSUM_SIZE);
            Stream msCompressedContent = new MemoryStream(bytesToRead);
            CopyStream(binReader.BaseStream, msCompressedContent, bytesToRead);

            // Read Adler32 checksum from archive
            var adler32Checksum = binReader.ReadUInt32();

            // Get uncompressed content
            var msUncompressedContent = new MemoryStream(itemHeader.SizeUncompressed);
            msCompressedContent.Position = 0;
            using (Stream deflateStream = new DeflateStream(msCompressedContent, CompressionMode.Decompress, true))
            {
                CopyStream(deflateStream, msUncompressedContent);
            }

            // Get content as byte buffer
            msUncompressedContent.Position = 0;
            byte[] buffer = msUncompressedContent.GetBuffer();

            // Verify Adler-32 checksum
            byte[] adler32Bytes = ChecksumHelper.ComputeAdler32Checksum(buffer, 0, buffer.Length);
            var computedAdler32Checksum = BitConverter.ToUInt32(adler32Bytes, 0);
            if (computedAdler32Checksum != adler32Checksum)
            {
                throw new Exception("Error: Adler32 checksum mismatch");
            }

            // Verify CRC checksum
            byte[] crc32Bytes = ChecksumHelper.ComputeCRC32XChecksum(buffer, 0, buffer.Length);
            var computedCrc32Checksum = BitConverter.ToUInt32(crc32Bytes, 0);
            if (computedCrc32Checksum != itemHeader.Crc32Checksum)
            {
                throw new Exception("Error: CRC32 checksum mismatch");
            }

            // if traget file exists, make sure we can overwrite it
            if (File.Exists(destFileName))
            {
                File.SetAttributes(destFileName, FileAttributes.Normal);
            }

            // Write uncompressed content to file
            msUncompressedContent.Position = 0;
            using (var fileStreamOut = new FileStream(destFileName, FileMode.Create, FileAccess.Write))
            {
                CopyStream(msUncompressedContent, fileStreamOut);
            }

            // Set file last write time
            File.SetLastWriteTimeUtc(destFileName, itemHeader.LastModified);
        }


        // Copy data from stream to stream
        private static void CopyStream(Stream input, Stream output, int numBytes = 0)
        {
            // Lazy initialize buffer
            if (copyStreamBuffer == null)
            {
                copyStreamBuffer = new byte[COPYSTREAM_BUFFER_SIZE];
            }
            // NumBytes = 0 => copy everything
            if (numBytes == 0)
            {
                numBytes = int.MaxValue;
            }
            int numBytesRead = 0;
            while (numBytes > 0)
            {
                numBytesRead = input.Read(copyStreamBuffer, 0, Math.Min(copyStreamBuffer.Length, numBytes));
                if (numBytesRead > 0)
                {
                    output.Write(copyStreamBuffer, 0, numBytesRead);
                    numBytes -= numBytesRead;
                }
                else
                {
                    break;
                }
            }
        }


        // Extract archive item
        private static void ReadArchiveItemHeader(BinaryReader binReader, out ArchiveItemHeader itemHeader)
        {
            // Save current stream position
            int streamPosStart = (int)binReader.BaseStream.Position;

            // Read magic number
            var magicNumber = binReader.ReadUInt32();
            if (magicNumber != MVA_MAGICNUM_ARCHIVE_ITEM)
            {
                throw new Exception("Error: Invalid magic number in archive item header");
            }

            // Read version
            var version = binReader.ReadUInt16();
            if (version != 1)
            {
                throw new Exception("Error: Unsupported version in archive item header");
            }

            // data offset
            var offsetItemData = binReader.ReadUInt16();

            // file times
            var timeLastModified = binReader.ReadUInt32();
            var timeLastAccessed = binReader.ReadUInt32();

            // file name
            var filename = Encoding.ASCII.GetString(binReader.ReadBytes(MAX_PATH)).TrimEnd('\0');

            // sizes
            var sizeUncompressed = binReader.ReadUInt32();
            var sizeCompressed = binReader.ReadUInt32();

            // CRC32 checksum
            var crc32Checksum = binReader.ReadUInt32();

            // Read magic number(2)
            var magicNumber2 = binReader.ReadUInt32();
            if (magicNumber2 != MVA_MAGICNUM_ARCHIVE_ITEM_2)
            {
                throw new Exception("Error: Invalid magic number (2) in archive item header");
            }

            // Read magic number(3)
            var magicNumber3 = binReader.ReadUInt32();
            if (magicNumber3 != MVA_MAGICNUM_ARCHIVE_ITEM_3)
            {
                throw new Exception("Error: Invalid magic number (3) in archive item header");
            }

            // Skip to end of header
            int bytesToSkip = offsetItemData + streamPosStart - (int)binReader.BaseStream.Position;
            binReader.BaseStream.Seek(bytesToSkip, SeekOrigin.Current);

            // Store and return header info
            var epochStart = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
            itemHeader.FileName = filename;
            itemHeader.LastModified = epochStart.AddSeconds(timeLastModified);
            itemHeader.LastAccessed = epochStart.AddSeconds(timeLastAccessed);
            itemHeader.SizeCompressed = (int)sizeCompressed;
            itemHeader.SizeUncompressed = (int)sizeUncompressed;
            itemHeader.Crc32Checksum = crc32Checksum;
        }


        // Read archive header
        private static void ReadArchiveHeader(BinaryReader binReader)
        {
            // Save current stream position
            int streamPosStart = (int)binReader.BaseStream.Position;

            // Read magic number
            var magicNumber = binReader.ReadUInt32();
            if (magicNumber != MVA_MAGICNUM_ARCHIVE)
            {
                throw new Exception("Error: Invalid magic number in archive header");
            }

            // Read version
            var version = binReader.ReadUInt16();
            if (version != 1)
            {
                throw new Exception("Error: Unsupported version in archive header");
            }

            // Skip to end of header
            int bytesToSkip = MVA_ARCHIVE_HEADER_SIZE + streamPosStart - (int)binReader.BaseStream.Position;
            binReader.BaseStream.Seek(bytesToSkip, SeekOrigin.Current);
        }


        // Archive item header data struct
        private struct ArchiveItemHeader
        {
            internal string FileName;
            internal DateTime LastModified;
            internal DateTime LastAccessed;
            internal int SizeCompressed;
            internal int SizeUncompressed;
            internal uint Crc32Checksum;
        }
    }
}

