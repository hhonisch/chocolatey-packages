using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Linq;
using System.IO;
using System.Reflection;

namespace HcwInstallHelper.Tests
{
    [TestClass()]
    public class IniFileTests
    {
        private IniFile iniFile;
        private string testDataDir;

        // Create temp ini file (empty or from template)
        private IniFile CreateTempIniFile(string templateFile = null)
        {
            string random = Path.GetRandomFileName();
            string filename = Path.Combine(testDataDir, $"temp_{random}.ini");
            if (File.Exists(filename))
            {
                File.Delete(filename);
            }
            if (templateFile != null)
            {
                Assert.IsTrue(File.Exists(templateFile));
                File.Copy(templateFile, filename);
            }
            return new IniFile(filename);
        }

        // Clean temp ini files
        private void CleanTempIniFiles()
        {
            var files = Directory.GetFiles(testDataDir, "temp_*.ini");
            foreach (string file in files)
            {
                File.Delete(file);
            }
        }

        [TestInitialize]
        public void TestInitialize()
        {
            // Data dir for files
            testDataDir = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "TestData");
            // Cleanup old temp files
            CleanTempIniFiles();
            // Reference to test ini file
            string iniFileName = Path.Combine(testDataDir, "Test.ini");
            Assert.IsTrue(File.Exists(iniFileName), $"File not found: {iniFileName}");
            iniFile = new IniFile(iniFileName);
        }


        [TestMethod()]
        public void WriteStringTest()
        {
            var emptyIniFile = CreateTempIniFile();
            Assert.IsTrue(emptyIniFile.WriteString("section 1", "key 1", "section 1, key 1"));
            Assert.AreEqual("section 1, key 1", emptyIniFile.GetString("section 1", "key 1", ""));
        }


        [TestMethod()]
        public void GetStringTest()
        {
            Assert.AreEqual("", iniFile.GetString("nonexistent", "nonexistent", ""));
            Assert.AreEqual("123", iniFile.GetString("nonexistent", "nonexistent", "123"));
            Assert.AreEqual("section 1, key 1", iniFile.GetString("section 1", "key 1", ""));
        }

        [TestMethod()]
        public void GetSectionNamesTest()
        {
            string[] sectionNames = iniFile.GetSectionNames();
            Assert.IsTrue(sectionNames.Contains("section 0"));
            Assert.IsTrue(sectionNames.Contains("section 1"));
            Assert.IsTrue(sectionNames.Contains("section 2"));
            Assert.IsTrue(sectionNames.Contains("section 3"));
        }

        [TestMethod()]
        public void WriteSectionTest()
        {
            var emptyIniFile = CreateTempIniFile();
            var data = new string[]
            {
                "key 1=section 1, key 1",
                "key 2=section 1, key 2",
                "key 3=section 1, key 3",
            };
            Assert.IsTrue(emptyIniFile.WriteSection("section 1", data));
            string[] section = emptyIniFile.GetSection("section 1");
            Assert.IsTrue(section.Contains("key 1=section 1, key 1"));
            Assert.IsTrue(section.Contains("key 2=section 1, key 2"));
            Assert.IsTrue(section.Contains("key 3=section 1, key 3"));
        }
    }
}