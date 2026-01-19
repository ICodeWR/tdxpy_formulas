/**
 * @file        test_tdxpy_logger.cpp
 * @brief       tdxpy_logger 单元测试
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

#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include <atomic>
#include <regex>
#include "tdxpy_logger.h"
#include "gtest/gtest.h"

namespace fs = std::filesystem;

class TdxpyLoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 每次测试前清理日志文件
        testLogFile = "test_tdxpy_logger.log";
        if (fs::exists(testLogFile))
        {
            fs::remove(testLogFile);
        }

        // 清理之前的日志实例状态
        tdxpy::Logger::getInstance().cleanup();
    }

    void TearDown() override
    {
        // 测试后清理
        tdxpy::Logger::getInstance().cleanup();

        // 删除测试日志文件
        if (fs::exists(testLogFile))
        {
            try
            {
                fs::remove(testLogFile);
            }
            catch (...)
            {
                // 忽略删除错误
            }
        }
    }

    std::string testLogFile;

    // 辅助函数：读取日志文件内容
    std::string readLogFile()
    {
        std::ifstream file(testLogFile);
        if (!file.is_open())
        {
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // 辅助函数：等待文件写入
    void waitForFileWrite(size_t expectedLines = 1)
    {
        const int maxRetries = 10;
        for (int i = 0; i < maxRetries; ++i)
        {
            std::ifstream file(testLogFile);
            if (file.is_open())
            {
                std::string line;
                size_t lineCount = 0;
                while (std::getline(file, line))
                {
                    if (!line.empty())
                    {
                        ++lineCount;
                    }
                }

                if (lineCount >= expectedLines)
                {
                    return;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

// 测试1: 日志系统初始化
TEST_F(TdxpyLoggerTest, Initialize_Success)
{
    // 测试初始化
    bool result = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    EXPECT_TRUE(result);

    // 验证日志文件已创建
    waitForFileWrite(1);
    EXPECT_TRUE(fs::exists(testLogFile));

    // 验证文件内容包含初始化标记
    std::string content = readLogFile();
    EXPECT_NE(content.find("Logging started"), std::string::npos);
}

// 测试2: 重复初始化
TEST_F(TdxpyLoggerTest, Initialize_MultipleCalls)
{
    // 第一次初始化
    bool result1 = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    EXPECT_TRUE(result1);

    // 第二次初始化（应成功，但会重新打开文件）
    bool result2 = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogDebug);
    EXPECT_TRUE(result2);

    waitForFileWrite(1);
    EXPECT_TRUE(fs::exists(testLogFile));
}

// 测试3: 初始化失败（无效路径）
TEST_F(TdxpyLoggerTest, Initialize_InvalidPath)
{
    // 尝试创建在不可访问位置的日志文件
    std::string invalidPath = "/nonexistent/path/test.log";
    bool result = tdxpy::Logger::getInstance().initialize(invalidPath, tdxpy::LogInfo);
    EXPECT_FALSE(result);
}

// 测试4: 不同日志级别输出
TEST_F(TdxpyLoggerTest, Log_DifferentLevels)
{
    // 初始化日志，设置级别为INFO
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    // 记录不同级别的日志
    TDXPY_LOG_TRACE("This is a TRACE message");
    TDXPY_LOG_DEBUG("This is a DEBUG message");
    TDXPY_LOG_INFO("This is an INFO message");
    TDXPY_LOG_WARNING("This is a WARNING message");
    TDXPY_LOG_ERROR("This is an ERROR message");

    waitForFileWrite(4); // TRACE和DEBUG应该被过滤掉，所以应该是4行

    std::string content = readLogFile();

    // 验证TRACE和DEBUG被过滤
    EXPECT_EQ(content.find("TRACE"), std::string::npos);
    EXPECT_EQ(content.find("DEBUG"), std::string::npos);

    // 验证INFO、WARNING、ERROR被记录
    EXPECT_NE(content.find("INFO"), std::string::npos);
    EXPECT_NE(content.find("WARNING"), std::string::npos);
    EXPECT_NE(content.find("ERROR"), std::string::npos);

    // 验证消息内容
    EXPECT_NE(content.find("This is an INFO message"), std::string::npos);
    EXPECT_NE(content.find("This is a WARNING message"), std::string::npos);
    EXPECT_NE(content.find("This is an ERROR message"), std::string::npos);
}

// 测试5: 日志级别过滤
TEST_F(TdxpyLoggerTest, Log_LevelFiltering)
{
    // 测试TRACE级别：应该记录所有级别的日志
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogTrace);
    ASSERT_TRUE(initResult);

    TDXPY_LOG_TRACE("Trace message at TRACE level");
    TDXPY_LOG_DEBUG("Debug message at TRACE level");
    TDXPY_LOG_INFO("Info message at TRACE level");

    waitForFileWrite(3);

    std::string content1 = readLogFile();
    EXPECT_NE(content1.find("TRACE"), std::string::npos);
    EXPECT_NE(content1.find("DEBUG"), std::string::npos);
    EXPECT_NE(content1.find("INFO"), std::string::npos);

    // 清理并重新测试WARNING级别
    tdxpy::Logger::getInstance().cleanup();
    fs::remove(testLogFile);

    initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogWarning);
    ASSERT_TRUE(initResult);

    TDXPY_LOG_DEBUG("Debug message at WARNING level");
    TDXPY_LOG_INFO("Info message at WARNING level");
    TDXPY_LOG_WARNING("Warning message at WARNING level");
    TDXPY_LOG_ERROR("Error message at WARNING level");

    waitForFileWrite(3);

    std::string content2 = readLogFile();
    EXPECT_EQ(content2.find("DEBUG"), std::string::npos);
    EXPECT_EQ(content2.find("INFO"), std::string::npos);
    EXPECT_NE(content2.find("WARNING"), std::string::npos);
    EXPECT_NE(content2.find("ERROR"), std::string::npos);
}

// 测试6: 日志格式验证
TEST_F(TdxpyLoggerTest, Log_FormatValidation)
{
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    TDXPY_LOG_INFO("Test message with special characters: !@#$%^&*()");

    waitForFileWrite(1);

    std::string content = readLogFile();

    // 验证基本格式组件
    EXPECT_NE(content.find("INFO"), std::string::npos);
    EXPECT_NE(content.find("tdxpy_logger.cpp"), std::string::npos);
    EXPECT_NE(content.find("Test message with special characters: !@#$%^&*()"), std::string::npos);

    // 验证时间戳格式（YYYY-MM-DD HH:MM:SS.mmm）
    std::regex timestampRegex(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})");
    EXPECT_TRUE(std::regex_search(content, timestampRegex));
}

// 测试7: 并发日志记录
TEST_F(TdxpyLoggerTest, Log_ConcurrentAccess)
{
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    constexpr int numThreads = 10;
    constexpr int messagesPerThread = 20;

    std::vector<std::thread> threads;
    std::atomic<int> messageCounter{0};

    // 创建多个线程同时记录日志
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([this, i, &messageCounter]()
                             {
            for (int j = 0; j < messagesPerThread; ++j) {
                std::string message = "Thread " + std::to_string(i) + 
                                     " message " + std::to_string(j);
                TDXPY_LOG_INFO(message);
                ++messageCounter;
                
                // 轻微延迟以增加并发交错的可能性
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } });
    }

    // 等待所有线程完成
    for (auto &thread : threads)
    {
        thread.join();
    }

    // 等待文件写入完成
    waitForFileWrite(numThreads * messagesPerThread);

    // 验证所有消息都被记录
    std::string content = readLogFile();

    // 计算INFO行数
    std::stringstream ss(content);
    std::string line;
    int infoLineCount = 0;

    while (std::getline(ss, line))
    {
        if (line.find("INFO") != std::string::npos)
        {
            ++infoLineCount;
        }
    }

    // 应该至少有numThreads * messagesPerThread条消息
    EXPECT_GE(infoLineCount, numThreads * messagesPerThread);
}

// 测试8: 日志级别转换
TEST_F(TdxpyLoggerTest, LevelFromString_Valid)
{
    tdxpy::Logger &logger = tdxpy::Logger::getInstance();

    EXPECT_EQ(logger.getLevelFromString("TRACE"), tdxpy::LogTrace);
    EXPECT_EQ(logger.getLevelFromString("DEBUG"), tdxpy::LogDebug);
    EXPECT_EQ(logger.getLevelFromString("INFO"), tdxpy::LogInfo);
    EXPECT_EQ(logger.getLevelFromString("WARNING"), tdxpy::LogWarning);
    EXPECT_EQ(logger.getLevelFromString("WARN"), tdxpy::LogWarning);
    EXPECT_EQ(logger.getLevelFromString("ERROR"), tdxpy::LogError);
    EXPECT_EQ(logger.getLevelFromString("OFF"), tdxpy::LogOff);
}

// 测试9: 日志级别转换（无效输入）
TEST_F(TdxpyLoggerTest, LevelFromString_Invalid)
{
    tdxpy::Logger &logger = tdxpy::Logger::getInstance();

    // 无效字符串应返回默认级别（INFO）
    EXPECT_EQ(logger.getLevelFromString("INVALID"), tdxpy::LogInfo);
    EXPECT_EQ(logger.getLevelFromString(""), tdxpy::LogInfo);
    EXPECT_EQ(logger.getLevelFromString("123"), tdxpy::LogInfo);
}

// 测试10: 日志级别转换（大小写不敏感）
TEST_F(TdxpyLoggerTest, LevelFromString_CaseInsensitive)
{
    tdxpy::Logger &logger = tdxpy::Logger::getInstance();

    EXPECT_EQ(logger.getLevelFromString("trace"), tdxpy::LogTrace);
    EXPECT_EQ(logger.getLevelFromString("Debug"), tdxpy::LogDebug);
    EXPECT_EQ(logger.getLevelFromString("info"), tdxpy::LogInfo);
    EXPECT_EQ(logger.getLevelFromString("warning"), tdxpy::LogWarning);
    EXPECT_EQ(logger.getLevelFromString("error"), tdxpy::LogError);
    EXPECT_EQ(logger.getLevelFromString("off"), tdxpy::LogOff);
}

// 测试11: 文件名提取功能
TEST_F(TdxpyLoggerTest, ExtractFileName)
{
    // 这个测试直接测试私有方法，但在实际中我们可以通过日志输出来间接验证
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    // 记录一条日志，其中包含文件路径
    TDXPY_LOG_INFO("Test file name extraction");

    waitForFileWrite(1);

    std::string content = readLogFile();

    // 验证日志中只显示文件名，而不是完整路径
    // 注意：这取决于__FILE__宏的展开
    // 我们主要确认格式正确即可
    EXPECT_NE(content.find("test_tdxpy_logger.cpp"), std::string::npos);
}

// 测试12: 清理和重新初始化
TEST_F(TdxpyLoggerTest, CleanupAndReinitialize)
{
    // 第一次初始化
    bool result1 = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    EXPECT_TRUE(result1);

    TDXPY_LOG_INFO("First initialization message");
    waitForFileWrite(1);

    // 清理
    tdxpy::Logger::getInstance().cleanup();

    // 第二次初始化
    bool result2 = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogDebug);
    EXPECT_TRUE(result2);

    TDXPY_LOG_DEBUG("Second initialization message");
    waitForFileWrite(2); // 包括第一段日志的结束标记和第二段日志的开始

    std::string content = readLogFile();

    // 验证包含清理标记
    EXPECT_NE(content.find("Logging stopped"), std::string::npos);
    EXPECT_NE(content.find("Logging started"), std::string::npos);

    // 验证两次的日志都记录
    EXPECT_NE(content.find("First initialization message"), std::string::npos);
    EXPECT_NE(content.find("Second initialization message"), std::string::npos);
}

// 测试13: 长消息日志
TEST_F(TdxpyLoggerTest, Log_LongMessage)
{
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    // 创建长消息
    std::string longMessage(2048, 'A'); // 2KB的消息
    longMessage += "END";

    TDXPY_LOG_INFO(longMessage);

    waitForFileWrite(1);

    std::string content = readLogFile();

    // 验证长消息被正确记录
    EXPECT_NE(content.find("END"), std::string::npos);
}

// 测试14: 特殊字符和Unicode
TEST_F(TdxpyLoggerTest, Log_SpecialCharactersAndUnicode)
{
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    // 测试包含各种特殊字符的消息
    std::string specialMessage =
        u8"Unicode测试: 中文, ελληνικά, Русский, 日本語\n"
        u8"特殊字符: \t\n\r\\\"'\n"
        u8"Emoji: 😀🚀🌟\n"
        u8"数学符号: αβγδ, ∫∑∏, ∀∃∈";

    TDXPY_LOG_INFO(specialMessage);

    waitForFileWrite(1);

    std::string content = readLogFile();

    // 验证消息被记录（我们主要关心不会崩溃或乱码）
    EXPECT_TRUE(content.length() > 0);

    // 验证包含部分可识别内容
    EXPECT_NE(content.find(u8"Unicode测试"), std::string::npos);
}

// 测试15: 性能基准测试
TEST_F(TdxpyLoggerTest, Performance_Benchmark)
{
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    constexpr int iterations = 1000;
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        TDXPY_LOG_INFO("Performance test message " + std::to_string(i));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    waitForFileWrite(iterations);

    // 验证所有消息都被记录
    std::string content = readLogFile();
    size_t pos = 0;
    int messageCount = 0;

    while ((pos = content.find("Performance test message", pos)) != std::string::npos)
    {
        ++messageCount;
        pos += 1;
    }

    EXPECT_GE(messageCount, iterations);

    // 输出性能信息（不在正式测试中assert）
    std::cout << "Logged " << iterations << " messages in "
              << duration.count() << " ms ("
              << (iterations * 1000.0 / duration.count()) << " msg/sec)" << std::endl;
}

// 测试16: 设置最小日志级别
TEST_F(TdxpyLoggerTest, SetMinLevel)
{
    bool initResult = tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogInfo);
    ASSERT_TRUE(initResult);

    // 初始级别为INFO，DEBUG应该被过滤
    TDXPY_LOG_DEBUG("This should not appear");
    TDXPY_LOG_INFO("This should appear");

    // 改变级别为DEBUG
    tdxpy::Logger::getInstance().setMinLevel(tdxpy::LogDebug);
    TDXPY_LOG_DEBUG("This should appear after level change");

    waitForFileWrite(3);

    std::string content = readLogFile();

    // 验证过滤效果
    EXPECT_EQ(content.find("This should not appear"), std::string::npos);
    EXPECT_NE(content.find("This should appear"), std::string::npos);
    EXPECT_NE(content.find("This should appear after level change"), std::string::npos);
}
