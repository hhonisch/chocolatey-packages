using HcwInstallHelper;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace HcwInstallHelper.Tests
{
    [TestClass()]
    public class HelperUtilsTests
    {
        [TestMethod()]
        public void GetSpecialFolderPathTest()
        {
            String commonPrograms = HelperUtils.GetSpecialFolderPath(HelperUtils.CSIDL.CSIDL_COMMON_PROGRAMS);
            String commonProgramsExpected = Environment.GetFolderPath(Environment.SpecialFolder.CommonPrograms);
            Assert.AreEqual(commonProgramsExpected, commonPrograms);

        }

        [TestMethod()]
        public void EnsureTrailingPathDelimiterTest()
        {
            Assert.AreEqual(@"C:\Temp\", HelperUtils.EnsureTrailingPathDelimiter(@"C:\Temp"));
            Assert.AreEqual(@"C:\Temp\", HelperUtils.EnsureTrailingPathDelimiter(@"C:\Temp\"));
            Assert.AreEqual(@"\", HelperUtils.EnsureTrailingPathDelimiter(@""));
            Assert.AreEqual(@"C:/Temp/", HelperUtils.EnsureTrailingPathDelimiter(@"C:/Temp"));
            Assert.AreEqual(@"C:/Temp/", HelperUtils.EnsureTrailingPathDelimiter(@"C:/Temp/"));
        }
    }
}