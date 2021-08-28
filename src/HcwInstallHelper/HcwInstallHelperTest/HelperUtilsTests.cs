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
            String commonPrograms = HelperUtils.GetSpecialFolderPath(HelperUtils.CSIDL.CSIDL_COMMON_PROGRAMS);
            String commonProgramsExpected = Environment.GetFolderPath(Environment.SpecialFolder.CommonPrograms);
            Assert.AreEqual(commonProgramsExpected, commonPrograms);

        }
    }
}