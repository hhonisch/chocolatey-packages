using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Reflection;
using System.Xml;

namespace HcwInstallHelper.Tests
{
    [TestClass()]
    public class ChecksumHelperTests
    {
        // Dir for test data
        private string testDataDir;
        // Checksum list doc
        private XmlDocument checkSumListDoc;

        private static TestContext testContext;

        [ClassInitialize]
        public static void ClassInitialize(TestContext testContext)
        {
            ChecksumHelperTests.testContext = testContext;
        }

        [TestInitialize]
        public void TestInitialize()
        {
            // Data dir for files
            testDataDir = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "TestData");

            // Load xml file containing checksum list
            string checksumListFileName = Path.Combine(testDataDir, "Checksums.xml");
            Assert.IsTrue(File.Exists(checksumListFileName), $"File not found: {checksumListFileName}");
            checkSumListDoc = new XmlDocument();
            checkSumListDoc.Load(checksumListFileName);
        }


        [TestMethod()]
        public void ComputeAdler32ChecksumTest()
        {
            // Process files in list
            var nodes = checkSumListDoc.SelectNodes("/files/file");
            foreach (XmlNode node in nodes)
            {
                // Get filename
                var filename = node.SelectSingleNode("filename").InnerText;
                testContext.Write($"Processing {filename}");

                // Get expected checksum
                var checksumExpected = node.SelectSingleNode("adler32").InnerText;

                // Load data from file
                var fileData = File.ReadAllBytes(Path.Combine(testDataDir, filename));

                // Verify checksums for complete data (case insensitive)
                var checksumActual = BitConverter.ToString(ChecksumHelper.ComputeAdler32Checksum(fileData, 0, fileData.Length));
                testContext.WriteLine("  Verify that checksums for complete data match");
                Assert.AreEqual(checksumExpected, checksumActual, true);

                // Verify checksums for truncated data (case insensitive)
                checksumActual = BitConverter.ToString(ChecksumHelper.ComputeAdler32Checksum(fileData, 0, fileData.Length - 1));
                testContext.WriteLine("  Verify that checksums for truncated data don't match");
                Assert.AreNotEqual(checksumExpected, checksumActual, true);
            }
        }

        [TestMethod()]
        public void ComputeCRC32XChecksumTest()
        {
            // Process files in list
            var nodes = checkSumListDoc.SelectNodes("/files/file");
            foreach (XmlNode node in nodes)
            {
                // Get filename
                var filename = node.SelectSingleNode("filename").InnerText;
                testContext.Write($"Processing {filename}");

                // Get expected checksum
                var checksumExpected = node.SelectSingleNode("crc32x").InnerText;

                // Load data from file
                var fileData = File.ReadAllBytes(Path.Combine(testDataDir, filename));

                // Verify checksums for complete data (case insensitive)
                var checksumActual = BitConverter.ToString(ChecksumHelper.ComputeCRC32XChecksum(fileData, 0, fileData.Length));
                testContext.WriteLine("  Verify that checksums for complete data match");
                Assert.AreEqual(checksumExpected, checksumActual, true);

                // Verify checksums for truncated data (case insensitive)
                checksumActual = BitConverter.ToString(ChecksumHelper.ComputeCRC32XChecksum(fileData, 0, fileData.Length - 1));
                testContext.WriteLine("  Verify that checksums for truncated data don't match");
                Assert.AreNotEqual(checksumExpected, checksumActual, true);
            }
        }
    }
}