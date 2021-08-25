using HcwInstallHelper;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace HcwInstallHelper.Tests
{
    [TestClass()]
    public class UtilsTests
    {
        [TestMethod()]
        public void GetSpecialFolderPathTest()
        {
            String commonPrograms = HelperUtils.GetSpecialFolderPath(0x17);
            String commonProgramsExpected = Environment.GetFolderPath(Environment.SpecialFolder.CommonPrograms);
            Assert.AreEqual(commonProgramsExpected, commonPrograms);

        }
    }
}