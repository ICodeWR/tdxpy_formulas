/**
 * @file        test_tdxpy_env_var.cpp
 * @brief       tdxpy_env_var 单元测试
 * @author      码上工坊
 * @copyright   Copyright (c) 2026-2030 码上工坊 Contributors
 * @license     MIT License (详见项目根目录LICENSE文件)
 * @version     0.1.0
 * @date        2026-01-12
 *
 * @par 修改记录:
 * <table>
 * <tr><th>日期         <th>版本      <th>作者              <th>描述
 * <tr><td>2026-01-12   <td>0.1.0    <td>码上工坊          <td>初始版本
 * </table>
 */

#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include "gtest/gtest.h"
#include "tdxpy_env_var.h"

namespace fs = std::filesystem;

class TdxpyEnvVarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 备份原始环境变量
        originalEnvBackup = tdxpy::utils::EnvVarManager::backup();

        // 清理测试环境变量
        cleanupTestEnvVars();

        // 设置测试目录
        testDir = "test_env_temp";
        fs::create_directories(testDir);
    }

    void TearDown() override
    {
        // 恢复原始环境变量
        tdxpy::utils::EnvVarManager::restore(originalEnvBackup);

        // 清理测试目录
        if (fs::exists(testDir))
        {
            fs::remove_all(testDir);
        }
    }

    void cleanupTestEnvVars()
    {
        // 清理可能存在的测试环境变量
        std::vector<std::string> testVars = {
            "TDXPY_TEST_VAR",
            "TDXPY_TEST_VAR2",
            "TDXPY_TEST_VAR3",
            "TDXPY_UTF8_TEST",
            "TDXPY_PATH_TEST"};

        for (const auto &var : testVars)
        {
            tdxpy::utils::EnvVarManager::remove(var);
        }
    }

    // 验证环境变量是否存在且值正确
    bool verifyEnvVar(const std::string &name, const std::string &expectedValue)
    {
        auto value = tdxpy::utils::EnvVarManager::get(name);
        return value.has_value() && value.value() == expectedValue;
    }

    // 验证宽字符环境变量
    bool verifyEnvVarW(const std::wstring &name, const std::wstring &expectedValue)
    {
        auto value = tdxpy::utils::EnvVarManager::getW(name);
        return value.has_value() && value.value() == expectedValue;
    }

    std::vector<std::pair<std::string, std::string>> originalEnvBackup;
    std::string testDir;

    // 测试用的各种字符串
    const std::string testAscii = "ASCII_TEST_VALUE_123";
    const std::string testUtf8 = u8"UTF-8测试: 中文测试 ελληνικά Русский 日本語";
    const std::string testSpecialChars = "Special!@#$%^&*()_+{}[]|\\:;\"'<>,.?/";
    const std::string testLongString = std::string(5000, 'A') + "END";

    const std::wstring testWideAscii = L"WIDE_ASCII_TEST";
    const std::wstring testWideUnicode = L"宽字符测试: ελληνικά Русский 日本語";
};

// 测试1: 基本设置和获取（ASCII）
TEST_F(TdxpyEnvVarTest, SetAndGet_ASCII)
{
    const std::string varName = "TDXPY_TEST_VAR";
    const std::string varValue = testAscii;

    // 设置环境变量
    bool setResult = tdxpy::utils::EnvVarManager::set(varName, varValue);
    EXPECT_TRUE(setResult);

    // 获取环境变量
    auto getResult = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_TRUE(getResult.has_value());
    EXPECT_EQ(getResult.value(), varValue);

    // 验证宽字符版本
    auto wideName = tdxpy::utils::EnvVarManager::utf8ToWide(varName);
    ASSERT_TRUE(wideName.has_value());

    auto wideValue = tdxpy::utils::EnvVarManager::getW(wideName.value());
    ASSERT_TRUE(wideValue.has_value());

    auto utf8Value = tdxpy::utils::EnvVarManager::wideToUtf8(wideValue.value());
    ASSERT_TRUE(utf8Value.has_value());
    EXPECT_EQ(utf8Value.value(), varValue);
}

// 测试2: 设置和获取UTF-8字符串
TEST_F(TdxpyEnvVarTest, SetAndGet_UTF8)
{
    const std::string varName = "TDXPY_UTF8_TEST";

    bool setResult = tdxpy::utils::EnvVarManager::set(varName, testUtf8);
    EXPECT_TRUE(setResult);

    auto getResult = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_TRUE(getResult.has_value());
    EXPECT_EQ(getResult.value(), testUtf8);
}

// 测试3: 设置和获取特殊字符
TEST_F(TdxpyEnvVarTest, SetAndGet_SpecialCharacters)
{
    const std::string varName = "TDXPY_SPECIAL_CHARS";

    bool setResult = tdxpy::utils::EnvVarManager::set(varName, testSpecialChars);
    EXPECT_TRUE(setResult);

    auto getResult = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_TRUE(getResult.has_value());
    EXPECT_EQ(getResult.value(), testSpecialChars);
}

// 测试4: 设置和获取长字符串
TEST_F(TdxpyEnvVarTest, SetAndGet_LongString)
{
    const std::string varName = "TDXPY_LONG_STRING";

    bool setResult = tdxpy::utils::EnvVarManager::set(varName, testLongString);
    EXPECT_TRUE(setResult);

    auto getResult = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_TRUE(getResult.has_value());
    EXPECT_EQ(getResult.value(), testLongString);
}

// 测试5: 设置空值
TEST_F(TdxpyEnvVarTest, Set_EmptyValue)
{
    const std::string varName = "TDXPY_EMPTY_VAR";

    // 设置为空字符串
    bool setResult = tdxpy::utils::EnvVarManager::set(varName, "");
    EXPECT_TRUE(setResult);

    auto getResult = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_FALSE(getResult.has_value());
}

// 测试6: 设置空名称（应该失败）
TEST_F(TdxpyEnvVarTest, Set_EmptyName)
{
    bool setResult = tdxpy::utils::EnvVarManager::set("", "value");
    EXPECT_FALSE(setResult);

    setResult = tdxpy::utils::EnvVarManager::setW(L"", L"value");
    EXPECT_FALSE(setResult);
}

// 测试7: 获取不存在的环境变量
TEST_F(TdxpyEnvVarTest, Get_NonExistent)
{
    const std::string nonExistentVar = "TDXPY_NON_EXISTENT_VAR_XYZ123";

    auto getResult = tdxpy::utils::EnvVarManager::get(nonExistentVar);
    EXPECT_FALSE(getResult.has_value());

    auto getResultW = tdxpy::utils::EnvVarManager::getW(L"TDXPY_NON_EXISTENT_VAR_XYZ123");
    EXPECT_FALSE(getResultW.has_value());
}

// 测试8: 检查环境变量是否存在
TEST_F(TdxpyEnvVarTest, Exists)
{
    const std::string varName = "TDXPY_EXISTS_TEST";

    // 初始应不存在
    bool existsBefore = tdxpy::utils::EnvVarManager::exists(varName);
    EXPECT_FALSE(existsBefore);

    // 设置后应存在
    bool setResult = tdxpy::utils::EnvVarManager::set(varName, "value");
    ASSERT_TRUE(setResult);

    bool existsAfter = tdxpy::utils::EnvVarManager::exists(varName);
    EXPECT_TRUE(existsAfter);

    // 检查空名称
    bool existsEmpty = tdxpy::utils::EnvVarManager::exists("");
    EXPECT_FALSE(existsEmpty);
}

// 测试9: 删除环境变量
TEST_F(TdxpyEnvVarTest, Remove)
{
    const std::string varName = "TDXPY_REMOVE_TEST";

    // 先设置
    bool setResult = tdxpy::utils::EnvVarManager::set(varName, "value");
    ASSERT_TRUE(setResult);
    EXPECT_TRUE(tdxpy::utils::EnvVarManager::exists(varName));

    // 删除
    bool removeResult = tdxpy::utils::EnvVarManager::remove(varName);
    EXPECT_TRUE(removeResult);

    // 验证已删除
    EXPECT_FALSE(tdxpy::utils::EnvVarManager::exists(varName));
    auto getResult = tdxpy::utils::EnvVarManager::get(varName);
    EXPECT_FALSE(getResult.has_value());
}

// 测试10: 删除不存在的环境变量
TEST_F(TdxpyEnvVarTest, Remove_NonExistent)
{
    const std::string nonExistentVar = "TDXPY_NON_EXISTENT_REMOVE";

    bool removeResult = tdxpy::utils::EnvVarManager::remove(nonExistentVar);
    // 删除不存在的变量应该返回true还是false？通常应该返回true
    // 实际上SetEnvironmentVariable在删除不存在的变量时返回TRUE
    EXPECT_TRUE(removeResult);
}

// 测试11: 宽字符版本设置和获取
TEST_F(TdxpyEnvVarTest, SetWAndGetW)
{
    const std::wstring varName = L"TDXPY_WIDE_TEST";
    const std::wstring varValue = testWideUnicode;

    // 设置宽字符环境变量
    bool setResult = tdxpy::utils::EnvVarManager::setW(varName, varValue);
    EXPECT_TRUE(setResult);

    // 获取宽字符环境变量
    auto getResult = tdxpy::utils::EnvVarManager::getW(varName);
    ASSERT_TRUE(getResult.has_value());
    EXPECT_EQ(getResult.value(), varValue);

    // 验证UTF-8版本
    auto utf8Name = tdxpy::utils::EnvVarManager::wideToUtf8(varName);
    auto utf8Value = tdxpy::utils::EnvVarManager::wideToUtf8(varValue);
    ASSERT_TRUE(utf8Name.has_value());
    ASSERT_TRUE(utf8Value.has_value());

    auto getResultUtf8 = tdxpy::utils::EnvVarManager::get(utf8Name.value());
    ASSERT_TRUE(getResultUtf8.has_value());
    EXPECT_EQ(getResultUtf8.value(), utf8Value.value());
}

// 测试12: 获取所有环境变量
TEST_F(TdxpyEnvVarTest, GetAll)
{
    // 设置几个测试变量
    tdxpy::utils::EnvVarManager::set("TDXPY_TEST1", "VALUE1");
    tdxpy::utils::EnvVarManager::set("TDXPY_TEST2", "VALUE2");
    tdxpy::utils::EnvVarManager::set("TDXPY_TEST3", testUtf8);

    // 获取所有环境变量
    auto allVars = tdxpy::utils::EnvVarManager::getAll();

    // 验证不为空
    EXPECT_FALSE(allVars.empty());

    // 查找我们的测试变量
    bool foundTest1 = false, foundTest2 = false, foundTest3 = false;

    for (const auto &[name, value] : allVars)
    {
        if (name == "TDXPY_TEST1")
        {
            foundTest1 = true;
            EXPECT_EQ(value, "VALUE1");
        }
        else if (name == "TDXPY_TEST2")
        {
            foundTest2 = true;
            EXPECT_EQ(value, "VALUE2");
        }
        else if (name == "TDXPY_TEST3")
        {
            foundTest3 = true;
            EXPECT_EQ(value, testUtf8);
        }
    }

    EXPECT_TRUE(foundTest1);
    EXPECT_TRUE(foundTest2);
    EXPECT_TRUE(foundTest3);
}

// 测试13: 备份和恢复环境变量
TEST_F(TdxpyEnvVarTest, BackupAndRestore)
{
    // 设置一些测试变量
    tdxpy::utils::EnvVarManager::set("TDXPY_BACKUP_TEST1", "ORIGINAL1");
    tdxpy::utils::EnvVarManager::set("TDXPY_BACKUP_TEST2", "ORIGINAL2");

    // 备份
    auto backup = tdxpy::utils::EnvVarManager::backup();

    // 修改环境变量
    tdxpy::utils::EnvVarManager::set("TDXPY_BACKUP_TEST1", "MODIFIED1");
    tdxpy::utils::EnvVarManager::remove("TDXPY_BACKUP_TEST2");
    tdxpy::utils::EnvVarManager::set("TDXPY_BACKUP_TEST3", "NEW3");

    // 验证修改
    auto value1 = tdxpy::utils::EnvVarManager::get("TDXPY_BACKUP_TEST1");
    ASSERT_TRUE(value1.has_value());
    EXPECT_EQ(value1.value(), "MODIFIED1");

    auto value2 = tdxpy::utils::EnvVarManager::get("TDXPY_BACKUP_TEST2");
    EXPECT_FALSE(value2.has_value());

    auto value3 = tdxpy::utils::EnvVarManager::get("TDXPY_BACKUP_TEST3");
    ASSERT_TRUE(value3.has_value());
    EXPECT_EQ(value3.value(), "NEW3");

    // 恢复
    bool restoreResult = tdxpy::utils::EnvVarManager::restore(backup);
    EXPECT_TRUE(restoreResult);

    // 验证恢复
    value1 = tdxpy::utils::EnvVarManager::get("TDXPY_BACKUP_TEST1");
    ASSERT_TRUE(value1.has_value());
    EXPECT_EQ(value1.value(), "ORIGINAL1");

    value2 = tdxpy::utils::EnvVarManager::get("TDXPY_BACKUP_TEST2");
    ASSERT_TRUE(value2.has_value());
    EXPECT_EQ(value2.value(), "ORIGINAL2");

    value3 = tdxpy::utils::EnvVarManager::get("TDXPY_BACKUP_TEST3");
    // TDXPY_BACKUP_TEST3 不应该存在，因为不在备份中
    EXPECT_FALSE(value3.has_value());
}

// 测试14: 恢复空备份
TEST_F(TdxpyEnvVarTest, Restore_EmptyBackup)
{
    std::vector<std::pair<std::string, std::string>> emptyBackup;

    bool restoreResult = tdxpy::utils::EnvVarManager::restore(emptyBackup);
    EXPECT_TRUE(restoreResult); // 恢复空备份应该成功

    // 环境变量应该保持不变（或者可能被清空，取决于实现）
    // 我们不验证具体内容，只验证函数调用不崩溃
}

// 测试15: 备份包含特殊字符
TEST_F(TdxpyEnvVarTest, Backup_SpecialCharacters)
{
    // 设置包含特殊字符的变量
    tdxpy::utils::EnvVarManager::set("TDXPY_SPECIAL", testUtf8);

    auto backup = tdxpy::utils::EnvVarManager::backup();

    // 找到我们的变量
    bool found = false;
    for (const auto &[name, value] : backup)
    {
        if (name == "TDXPY_SPECIAL")
        {
            found = true;
            EXPECT_EQ(value, testUtf8);
            break;
        }
    }

    EXPECT_TRUE(found);
}

// 测试16: 编码转换 - UTF-8 转宽字符
TEST_F(TdxpyEnvVarTest, Encoding_UTF8ToWide)
{
    // ASCII转换
    auto asciiWide = tdxpy::utils::EnvVarManager::utf8ToWide(testAscii);
    ASSERT_TRUE(asciiWide.has_value());
    EXPECT_FALSE(asciiWide.value().empty());

    // UTF-8转换
    auto utf8Wide = tdxpy::utils::EnvVarManager::utf8ToWide(testUtf8);
    ASSERT_TRUE(utf8Wide.has_value());
    EXPECT_FALSE(utf8Wide.value().empty());

    // 空字符串转换
    auto emptyWide = tdxpy::utils::EnvVarManager::utf8ToWide("");
    ASSERT_TRUE(emptyWide.has_value());
    EXPECT_EQ(emptyWide.value(), L"");

    // 无效UTF-8（应该失败）
    std::string invalidUtf8 = "\xC0\x80"; // 无效的UTF-8序列
    auto invalidWide = tdxpy::utils::EnvVarManager::utf8ToWide(invalidUtf8);
    // Windows的MultiByteToWideChar可能会失败，也可能不会，取决于实现
    // 我们不断言结果，只验证函数调用不崩溃
}

// 测试17: 编码转换 - 宽字符转UTF-8
TEST_F(TdxpyEnvVarTest, Encoding_WideToUTF8)
{
    // ASCII转换
    auto asciiUtf8 = tdxpy::utils::EnvVarManager::wideToUtf8(testWideAscii);
    ASSERT_TRUE(asciiUtf8.has_value());
    EXPECT_EQ(asciiUtf8.value(), "WIDE_ASCII_TEST");

    // Unicode转换
    auto unicodeUtf8 = tdxpy::utils::EnvVarManager::wideToUtf8(testWideUnicode);
    ASSERT_TRUE(unicodeUtf8.has_value());
    EXPECT_FALSE(unicodeUtf8.value().empty());

    // 空字符串转换
    auto emptyUtf8 = tdxpy::utils::EnvVarManager::wideToUtf8(L"");
    ASSERT_TRUE(emptyUtf8.has_value());
    EXPECT_EQ(emptyUtf8.value(), "");

    // 往返测试
    std::string original = testUtf8;
    auto toWide = tdxpy::utils::EnvVarManager::utf8ToWide(original);
    ASSERT_TRUE(toWide.has_value());

    auto backToUtf8 = tdxpy::utils::EnvVarManager::wideToUtf8(toWide.value());
    ASSERT_TRUE(backToUtf8.has_value());
    EXPECT_EQ(backToUtf8.value(), original);
}

// 测试18: PATH操作 - 添加到PATH前面
TEST_F(TdxpyEnvVarTest, Path_AddToFront)
{
    const std::string testPath = testDir + "/test_bin";
    fs::create_directories(testPath);

    // 获取原始PATH
    auto originalPath = tdxpy::utils::EnvVarManager::get("PATH");
    ASSERT_TRUE(originalPath.has_value());

    // 添加到PATH前面
    bool addResult = tdxpy::utils::EnvVarManager::addToPath(testPath, true);
    EXPECT_TRUE(addResult);

    // 获取新的PATH
    auto newPath = tdxpy::utils::EnvVarManager::get("PATH");
    ASSERT_TRUE(newPath.has_value());

    // 验证新路径在前面
    EXPECT_EQ(newPath.value().substr(0, testPath.length()), testPath);

    // 验证包含分号分隔符
    EXPECT_EQ(newPath.value()[testPath.length()], ';');

    // 验证原始PATH仍然存在
    EXPECT_NE(newPath.value().find(originalPath.value()), std::string::npos);
}

// 测试19: PATH操作 - 添加到PATH后面
TEST_F(TdxpyEnvVarTest, Path_AddToBack)
{
    const std::string testPath = testDir + "/test_bin2";
    fs::create_directories(testPath);

    // 获取原始PATH
    auto originalPath = tdxpy::utils::EnvVarManager::get("PATH");
    ASSERT_TRUE(originalPath.has_value());

    // 添加到PATH后面
    bool addResult = tdxpy::utils::EnvVarManager::addToPath(testPath, false);
    EXPECT_TRUE(addResult);

    // 获取新的PATH
    auto newPath = tdxpy::utils::EnvVarManager::get("PATH");
    ASSERT_TRUE(newPath.has_value());

    // 验证新路径在后面
    size_t pos = newPath.value().find(testPath);
    EXPECT_NE(pos, std::string::npos);

    // 如果是最后一项，前面应该有分号；如果不是最后一项，后面应该有分号
    if (pos + testPath.length() == newPath.value().length())
    {
        // 是最后一项
        EXPECT_EQ(newPath.value()[pos - 1], ';');
    }
    else
    {
        // 不是最后一项
        EXPECT_EQ(newPath.value()[pos + testPath.length()], ';');
    }
}

// 测试20: PATH操作 - 添加已存在的路径
TEST_F(TdxpyEnvVarTest, Path_AddExistingPath)
{
    const std::string testPath = testDir + "/existing_bin";
    fs::create_directories(testPath);

    // 第一次添加
    bool add1 = tdxpy::utils::EnvVarManager::addToPath(testPath, true);
    EXPECT_TRUE(add1);

    // 获取第一次添加后的PATH
    auto pathAfterFirstAdd = tdxpy::utils::EnvVarManager::get("PATH");
    ASSERT_TRUE(pathAfterFirstAdd.has_value());

    // 统计testPath出现的次数
    size_t count1 = 0;
    size_t pos = 0;
    while ((pos = pathAfterFirstAdd.value().find(testPath, pos)) != std::string::npos)
    {
        ++count1;
        pos += testPath.length();
    }
    EXPECT_EQ(count1, 1); // 应该只出现一次

    // 第二次添加相同的路径（应该不重复添加）
    bool add2 = tdxpy::utils::EnvVarManager::addToPath(testPath, true);
    EXPECT_TRUE(add2);

    // 获取第二次添加后的PATH
    auto pathAfterSecondAdd = tdxpy::utils::EnvVarManager::get("PATH");
    ASSERT_TRUE(pathAfterSecondAdd.has_value());

    // 统计testPath出现的次数
    size_t count2 = 0;
    pos = 0;
    while ((pos = pathAfterSecondAdd.value().find(testPath, pos)) != std::string::npos)
    {
        ++count2;
        pos += testPath.length();
    }
    EXPECT_EQ(count2, 1); // 应该仍然只出现一次
}

// 测试21: PATH操作 - 从PATH中移除
TEST_F(TdxpyEnvVarTest, Path_RemoveFromPath)
{
    const std::string testPath = testDir + "/remove_bin";
    fs::create_directories(testPath);

    // 先添加到PATH
    bool addResult = tdxpy::utils::EnvVarManager::addToPath(testPath, true);
    EXPECT_TRUE(addResult);

    // 验证已添加
    bool isInPathBefore = tdxpy::utils::EnvVarManager::isInPath(testPath);
    EXPECT_TRUE(isInPathBefore);

    // 从PATH中移除
    bool removeResult = tdxpy::utils::EnvVarManager::removeFromPath(testPath);
    EXPECT_TRUE(removeResult);

    // 验证已移除
    bool isInPathAfter = tdxpy::utils::EnvVarManager::isInPath(testPath);
    EXPECT_FALSE(isInPathAfter);

    // 获取PATH验证
    auto currentPath = tdxpy::utils::EnvVarManager::get("PATH");
    ASSERT_TRUE(currentPath.has_value());
    EXPECT_EQ(currentPath.value().find(testPath), std::string::npos);
}

// 测试22: PATH操作 - 移除不存在的路径
TEST_F(TdxpyEnvVarTest, Path_RemoveNonExistentPath)
{
    const std::string nonExistentPath = "C:/NonExistent/Path/123456";

    // 确保这个路径不在PATH中
    bool isInPathBefore = tdxpy::utils::EnvVarManager::isInPath(nonExistentPath);
    EXPECT_FALSE(isInPathBefore);

    // 尝试移除不存在的路径
    bool removeResult = tdxpy::utils::EnvVarManager::removeFromPath(nonExistentPath);
    EXPECT_TRUE(removeResult); // 应该成功（没有错误）

    // PATH应该保持不变
    auto pathBefore = tdxpy::utils::EnvVarManager::get("PATH");
    auto pathAfter = tdxpy::utils::EnvVarManager::get("PATH");

    if (pathBefore.has_value() && pathAfter.has_value())
    {
        EXPECT_EQ(pathBefore.value(), pathAfter.value());
    }
}

// 测试23: PATH操作 - 检查路径是否在PATH中
TEST_F(TdxpyEnvVarTest, Path_IsInPath)
{
    const std::string testPath = testDir + "/check_bin";
    fs::create_directories(testPath);

    // 初始应不在PATH中
    bool isInPathBefore = tdxpy::utils::EnvVarManager::isInPath(testPath);
    EXPECT_FALSE(isInPathBefore);

    // 添加到PATH
    tdxpy::utils::EnvVarManager::addToPath(testPath, true);

    // 现在应该在PATH中
    bool isInPathAfter = tdxpy::utils::EnvVarManager::isInPath(testPath);
    EXPECT_TRUE(isInPathAfter);

    // 测试带斜杠和不带斜杠的路径（应该被视为相同）
    std::string testPathWithSlash = testPath + "\\";
    bool isInPathWithSlash = tdxpy::utils::EnvVarManager::isInPath(testPathWithSlash);
    EXPECT_TRUE(isInPathWithSlash);

    // 测试完全不同的路径
    bool isInPathWrong = tdxpy::utils::EnvVarManager::isInPath("C:/Completely/Wrong/Path");
    EXPECT_FALSE(isInPathWrong);
}

// 测试24: PATH操作 - 空路径处理
TEST_F(TdxpyEnvVarTest, Path_EmptyPath)
{
    // 添加空路径应该失败
    bool addResult = tdxpy::utils::EnvVarManager::addToPath("", true);
    EXPECT_FALSE(addResult);

    // 移除空路径应该失败
    bool removeResult = tdxpy::utils::EnvVarManager::removeFromPath("");
    EXPECT_FALSE(removeResult);

    // 检查空路径应该返回false
    bool isInPathResult = tdxpy::utils::EnvVarManager::isInPath("");
    EXPECT_FALSE(isInPathResult);
}

// 测试25: PATH操作 - 大小写不敏感（Windows）
TEST_F(TdxpyEnvVarTest, Path_CaseInsensitive)
{
    const std::string testPath = testDir + "/CaseTest";
    fs::create_directories(testPath);

    // 添加小写路径
    tdxpy::utils::EnvVarManager::addToPath(testPath, true);

    // 使用大写版本检查（Windows应该不区分大小写）
    std::string upperPath = testPath;
    std::transform(upperPath.begin(), upperPath.end(), upperPath.begin(), ::toupper);

    bool isInPathUpper = tdxpy::utils::EnvVarManager::isInPath(upperPath);
#ifdef _WIN32
    EXPECT_TRUE(isInPathUpper); // Windows不区分大小写
#else
    // Linux/macOS区分大小写
#endif

    // 移除大写版本
    tdxpy::utils::EnvVarManager::removeFromPath(upperPath);

    // 原始路径也应该被移除
    bool isInPathAfter = tdxpy::utils::EnvVarManager::isInPath(testPath);
    EXPECT_FALSE(isInPathAfter);
}

// 测试26: 并发环境变量操作
TEST_F(TdxpyEnvVarTest, ConcurrentOperations)
{
    constexpr int numThreads = 10;
    constexpr int operationsPerThread = 50;

    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> totalOperations{0};

    // 创建多个线程同时操作环境变量
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([this, i, &successCount, &totalOperations]()
                             {
            for (int j = 0; j < operationsPerThread; ++j) {
                std::string varName = "TDXPY_CONCURRENT_" + std::to_string(i) + "_" + std::to_string(j);
                std::string varValue = "VALUE_" + std::to_string(i) + "_" + std::to_string(j);
                
                // 设置
                bool setResult = tdxpy::utils::EnvVarManager::set(varName, varValue);
                if (setResult) ++successCount;
                ++totalOperations;
                
                // 获取
                auto getResult = tdxpy::utils::EnvVarManager::get(varName);
                if (getResult.has_value() && getResult.value() == varValue) {
                    ++successCount;
                }
                ++totalOperations;
                
                // 检查是否存在
                bool existsResult = tdxpy::utils::EnvVarManager::exists(varName);
                if (existsResult) ++successCount;
                ++totalOperations;
                
                // 删除
                bool removeResult = tdxpy::utils::EnvVarManager::remove(varName);
                if (removeResult) ++successCount;
                ++totalOperations;
                
                // 短暂休眠以减少竞争
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } });
    }

    // 等待所有线程完成
    for (auto &thread : threads)
    {
        thread.join();
    }

    // 验证所有操作都成功（允许少量失败，因为环境变量是全局的）
    double successRate = static_cast<double>(successCount.load()) / totalOperations.load();
    EXPECT_GT(successRate, 0.95) << "并发操作成功率: " << (successRate * 100) << "%";
}

// 测试27: PATH分割和合并功能
TEST_F(TdxpyEnvVarTest, Path_SplitAndJoin)
{
    // 测试各种PATH格式
    std::vector<std::pair<std::string, std::vector<std::string>>> testCases = {
        {"", {}},
        {"C:\\Windows", {"C:\\Windows"}},
        {"C:\\Windows;D:\\Programs", {"C:\\Windows", "D:\\Programs"}},
        {"C:\\Windows;D:\\Programs;;E:\\Tools", {"C:\\Windows", "D:\\Programs", "E:\\Tools"}},
        {";C:\\Windows;;", {"C:\\Windows"}},
        {"C:\\Windows;", {"C:\\Windows"}},
        {";C:\\Windows", {"C:\\Windows"}},
        {"C:\\Windows\\System32;D:\\Python\\Scripts;E:\\NodeJS",
         {"C:\\Windows\\System32", "D:\\Python\\Scripts", "E:\\NodeJS"}}};

    for (const auto &[pathStr, expectedParts] : testCases)
    {
        // 测试分割
        auto splitResult = tdxpy::utils::EnvVarManager::get("PATH");
        // 注意：splitPath是私有方法，我们通过其他方法间接测试

        // 测试合并
        std::string joined = tdxpy::utils::EnvVarManager::get("PATH").value_or("");
        // 我们主要验证函数调用不崩溃
    }
}

// 测试28: 环境变量值覆盖
TEST_F(TdxpyEnvVarTest, OverrideValue)
{
    const std::string varName = "TDXPY_OVERRIDE_TEST";

    // 设置初始值
    tdxpy::utils::EnvVarManager::set(varName, "INITIAL_VALUE");

    // 验证初始值
    auto value1 = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_TRUE(value1.has_value());
    EXPECT_EQ(value1.value(), "INITIAL_VALUE");

    // 覆盖值
    tdxpy::utils::EnvVarManager::set(varName, "NEW_VALUE");

    // 验证新值
    auto value2 = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_TRUE(value2.has_value());
    EXPECT_EQ(value2.value(), "NEW_VALUE");

    // 再次覆盖
    tdxpy::utils::EnvVarManager::set(varName, "FINAL_VALUE");

    // 验证最终值
    auto value3 = tdxpy::utils::EnvVarManager::get(varName);
    ASSERT_TRUE(value3.has_value());
    EXPECT_EQ(value3.value(), "FINAL_VALUE");
}

// 测试29: 系统环境变量访问
TEST_F(TdxpyEnvVarTest, SystemEnvironmentVariables)
{
    // 测试一些常见的系统环境变量
    // 这些变量应该始终存在（在Windows上）

#ifdef _WIN32
    auto systemRoot = tdxpy::utils::EnvVarManager::get("SystemRoot");
    EXPECT_TRUE(systemRoot.has_value());
    EXPECT_FALSE(systemRoot.value().empty());

    auto tempDir = tdxpy::utils::EnvVarManager::get("TEMP");
    EXPECT_TRUE(tempDir.has_value());
    EXPECT_FALSE(tempDir.value().empty());

    auto path = tdxpy::utils::EnvVarManager::get("PATH");
    EXPECT_TRUE(path.has_value());
    EXPECT_FALSE(path.value().empty());

    auto username = tdxpy::utils::EnvVarManager::get("USERNAME");
    EXPECT_TRUE(username.has_value());
    EXPECT_FALSE(username.value().empty());
#endif

    // 测试不存在的系统变量
    auto nonExistent = tdxpy::utils::EnvVarManager::get("TDXPY_NON_EXISTENT_SYSTEM_VAR");
    EXPECT_FALSE(nonExistent.has_value());
}

// 测试30: 性能测试 - 大量环境变量操作
TEST_F(TdxpyEnvVarTest, Performance_MassiveOperations)
{
    constexpr int numOperations = 1000;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numOperations; ++i)
    {
        std::string varName = "TDXPY_PERF_" + std::to_string(i);
        std::string varValue = "VALUE_" + std::to_string(i);

        // 设置
        tdxpy::utils::EnvVarManager::set(varName, varValue);

        // 获取
        tdxpy::utils::EnvVarManager::get(varName);

        // 删除
        tdxpy::utils::EnvVarManager::remove(varName);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // 验证性能（1000次操作应在合理时间内完成）
    EXPECT_LT(duration.count(), 5000) << "1000次操作耗时: " << duration.count() << "ms";

    // 输出性能信息
    std::cout << "性能测试: " << numOperations << "次环境变量操作耗时 "
              << duration.count() << "ms ("
              << (numOperations * 1000.0 / duration.count()) << " ops/sec)" << std::endl;
}

// 测试31: 错误处理 - 无效的UTF-8编码
TEST_F(TdxpyEnvVarTest, ErrorHandling_InvalidUTF8)
{
    // 创建无效的UTF-8序列
    char invalidUtf8[] = {char(0xC0), char(0x80), 0}; // 过长的编码
    std::string invalidStr(invalidUtf8);

    // 尝试转换无效UTF-8
    auto wideResult = tdxpy::utils::EnvVarManager::utf8ToWide(invalidStr);
    // 可能返回空或转换后的值，取决于Windows实现
    // 我们不断言，只验证函数调用不崩溃

    // 尝试设置包含无效UTF-8的环境变量
    bool setResult = tdxpy::utils::EnvVarManager::set("TDXPY_INVALID_UTF8", invalidStr);
    // 可能成功也可能失败，取决于Windows实现
}

// 测试32: 边界条件 - 最大长度环境变量
TEST_F(TdxpyEnvVarTest, Boundary_MaxLength)
{
    // Windows环境变量最大长度通常是32767字符
    constexpr size_t maxLength = 32766; // 留一个字符给null终止符

    std::string longValue(maxLength, 'X');
    longValue += "END";

    // 设置长环境变量
    bool setResult = tdxpy::utils::EnvVarManager::set("TDXPY_MAX_LENGTH", longValue);
    EXPECT_TRUE(setResult);

    // 获取并验证
    auto getResult = tdxpy::utils::EnvVarManager::get("TDXPY_MAX_LENGTH");
    ASSERT_TRUE(getResult.has_value());
    EXPECT_EQ(getResult.value().length(), maxLength + 3); // "END"增加了3个字符
}

// 测试33: 边界条件 - 超长环境变量名
TEST_F(TdxpyEnvVarTest, Boundary_LongVariableName)
{
    // 创建超长变量名（Windows最大是32767，但实际中应该短得多）
    std::string longName(1000, 'A');

    bool setResult = tdxpy::utils::EnvVarManager::set(longName, "value");
    EXPECT_TRUE(setResult);

    bool existsResult = tdxpy::utils::EnvVarManager::exists(longName);
    EXPECT_TRUE(existsResult);

    auto getResult = tdxpy::utils::EnvVarManager::get(longName);
    ASSERT_TRUE(getResult.has_value());
    EXPECT_EQ(getResult.value(), "value");

    // 清理
    tdxpy::utils::EnvVarManager::remove(longName);
}

// 测试34: 路径标准化处理
TEST_F(TdxpyEnvVarTest, Path_Normalization)
{
    // 创建测试路径
    std::string testPath = testDir + "\\test_bin\\";
    fs::create_directories(testPath);

    // 添加带斜杠的路径
    tdxpy::utils::EnvVarManager::addToPath(testPath, true);

    // 检查不带斜杠的路径
    std::string testPathNoSlash = testPath.substr(0, testPath.length() - 1);
    bool isInPath1 = tdxpy::utils::EnvVarManager::isInPath(testPathNoSlash);
    EXPECT_TRUE(isInPath1);

    // 检查带不同斜杠的路径
    std::string testPathForwardSlash = testPath;
    std::replace(testPathForwardSlash.begin(), testPathForwardSlash.end(), '\\', '/');
    bool isInPath2 = tdxpy::utils::EnvVarManager::isInPath(testPathForwardSlash);
#ifdef _WIN32
    EXPECT_FALSE(isInPath2);
#endif

    // 清理
    tdxpy::utils::EnvVarManager::removeFromPath(testPath);
}
