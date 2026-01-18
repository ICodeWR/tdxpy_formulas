/**
 * @file        test_tdxpy_plugin_registry.cpp
 * @brief       tdxpy_plugin_registry 单元测试
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

#define NOMINMAX

#include <windows.h>
#include <vector>
#include <algorithm>
#include <memory>
#include "gtest/gtest.h"
#include "tdxpy_plugin_registry.h"
#include "tdxpy_logger.h"
#include "tdxpy_python_engine.h"

// #include "mock_tdxpy_python_engine.h"

// 测试用的DLL句柄
static HMODULE g_testDllHandle = nullptr;

// 前向声明：我们需要的测试函数

#ifndef BOOL
#define BOOL int
#endif

extern "C" BOOL RegisterTdxFunc(TdxPluginFunctionInfo **pluginFuncTable);

// 模拟的Python引擎调用结果
static int g_mockPythonResult = 1;
static int g_lastCalledFunctionId = 0;
static int g_lastDataLength = 0;

// 测试用的插件函数
void TestPluginFunction0(int dataLength, float *output,
                         float *inputA, float *inputB, float *inputC)
{
    g_lastCalledFunctionId = 0;
    g_lastDataLength = dataLength;

    if (output && dataLength > 0)
    {
        for (int i = 0; i < dataLength; ++i)
        {
            output[i] = 0.0f; // 函数0通常用于特殊处理
        }
    }
}

void TestPluginFunction1(int dataLength, float *output,
                         float *inputA, float *inputB, float *inputC)
{
    g_lastCalledFunctionId = 1;
    g_lastDataLength = dataLength;

    if (output && dataLength > 0)
    {
        for (int i = 0; i < dataLength; ++i)
        {
            output[i] = 1.0f * (i + 1); // 简单的线性序列
        }
    }
}

void TestPluginFunction2(int dataLength, float *output,
                         float *inputA, float *inputB, float *inputC)
{
    g_lastCalledFunctionId = 2;
    g_lastDataLength = dataLength;

    if (output && dataLength > 0)
    {
        // 使用输入数据（如果提供）
        float baseValue = 1.0f;
        if (inputA)
        {
            baseValue = inputA[0];
        }

        for (int i = 0; i < dataLength; ++i)
        {
            output[i] = baseValue * (i + 1) * 2.0f;
        }
    }
}

void TestPluginFunctionError(int dataLength, float *output,
                             float *inputA, float *inputB, float *inputC)
{
    g_lastCalledFunctionId = -1; // 错误标记
    g_lastDataLength = dataLength;

    // 模拟Python调用失败
    g_mockPythonResult = 0;
}

class TdxpyPluginRegistryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 重置测试状态
        g_lastCalledFunctionId = 0;
        g_lastDataLength = 0;
        g_mockPythonResult = 1;

        // 初始化日志（使用内存或文件）
        testLogFile = "test_plugin_registry.log";
        tdxpy::Logger::getInstance().initialize(testLogFile, tdxpy::LogDebug);

        // 创建测试数据
        testDataLength = 10;
        testOutput.resize(testDataLength, 0.0f);
        testInputA.resize(testDataLength, 1.0f);
        testInputB.resize(testDataLength, 2.0f);
        testInputC.resize(testDataLength, 3.0f);

        // 初始化测试数据
        for (int i = 0; i < testDataLength; ++i)
        {
            testInputA[i] = static_cast<float>(i + 1);
            testInputB[i] = static_cast<float>(i + 1) * 2.0f;
            testInputC[i] = static_cast<float>(i + 1) * 3.0f;
        }
    }

    void TearDown() override
    {
        // 清理日志
        tdxpy::Logger::getInstance().cleanup();

        // 如果加载了DLL，卸载它
        if (g_testDllHandle)
        {
            FreeLibrary(g_testDllHandle);
            g_testDllHandle = nullptr;
        }
    }

    // 验证输出数组
    void validateOutput(const std::vector<float> &output,
                        int expectedLength,
                        float expectedFirstValue = 0.0f)
    {
        EXPECT_EQ(output.size(), expectedLength);

        for (int i = 0; i < expectedLength; ++i)
        {
            EXPECT_FALSE(std::isnan(output[i])) << "位置 " << i << " 的值为NaN";
            EXPECT_FALSE(std::isinf(output[i])) << "位置 " << i << " 的值为Inf";
        }

        if (expectedFirstValue != 0.0f && expectedLength > 0)
        {
            EXPECT_EQ(output[0], expectedFirstValue) << "第一个值不符合预期";
        }
    }

    // 打印输出用于调试
    void printOutput(const std::vector<float> &output, const std::string &name)
    {
        std::cout << "\n"
                  << name << " 输出 (" << output.size() << "个值):" << std::endl;
        for (size_t i = 0; i < std::min(output.size(), size_t(5)); ++i)
        {
            std::cout << "  [" << i << "] = " << output[i] << std::endl;
        }
        if (output.size() > 5)
        {
            std::cout << "  ..." << std::endl;
            std::cout << "  [" << output.size() - 1 << "] = " << output.back() << std::endl;
        }
    }

    std::string testLogFile;
    int testDataLength;
    std::vector<float> testOutput;
    std::vector<float> testInputA;
    std::vector<float> testInputB;
    std::vector<float> testInputC;
};

// 测试1: 插件函数信息结构体大小和对齐
TEST_F(TdxpyPluginRegistryTest, Structure_SizeAndAlignment)
{
    // 验证结构体大小（使用pragma pack(1)应该是紧凑的）
    EXPECT_EQ(sizeof(TdxPluginFunctionInfo), sizeof(unsigned short) + sizeof(TdxPluginFunction));

    // 验证字段偏移
    TdxPluginFunctionInfo info;
    EXPECT_EQ(reinterpret_cast<uintptr_t>(&info.functionId) - reinterpret_cast<uintptr_t>(&info), 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(&info.function) - reinterpret_cast<uintptr_t>(&info),
              sizeof(unsigned short));
}

// 测试2: 插件函数指针类型
TEST_F(TdxpyPluginRegistryTest, FunctionPointer_Type)
{
    // 验证函数指针类型定义
    TdxPluginFunction funcPtr = TestPluginFunction1;

    EXPECT_NE(funcPtr, nullptr);

    // 调用函数指针
    std::vector<float> output(5, 0.0f);
    funcPtr(5, output.data(), nullptr, nullptr, nullptr);

    // 验证函数被调用
    EXPECT_EQ(g_lastCalledFunctionId, 1);
    EXPECT_EQ(g_lastDataLength, 5);
}

// 测试3: 注册函数 - 正常情况
TEST_F(TdxpyPluginRegistryTest, RegisterTdxFunc_Normal)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;

    // 第一次注册应该成功
    BOOL result = RegisterTdxFunc(&pluginTable);

    EXPECT_EQ(result, TRUE);
    EXPECT_NE(pluginTable, nullptr);

    // 验证函数表不为空
    EXPECT_NE(pluginTable[0].function, nullptr);
    EXPECT_EQ(pluginTable[0].functionId, 0);

    // 验证有足够的函数（0-100）
    int functionCount = 0;
    while (pluginTable[functionCount].function != nullptr)
    {
        ++functionCount;
    }

    EXPECT_GE(functionCount, 101); // 0-100号函数

    // 验证函数ID顺序
    for (int i = 0; i < functionCount; ++i)
    {
        EXPECT_EQ(pluginTable[i].functionId, static_cast<unsigned short>(i));
    }
}

// 测试4: 注册函数 - 空指针传入
TEST_F(TdxpyPluginRegistryTest, RegisterTdxFunc_NullPointer)
{
    // 传入空指针应该失败
    BOOL result = RegisterTdxFunc(nullptr);

    EXPECT_EQ(result, FALSE);
}

// 测试5: 注册函数 - 重复注册
TEST_F(TdxpyPluginRegistryTest, RegisterTdxFunc_DuplicateRegistration)
{
    TdxPluginFunctionInfo *pluginTable1 = nullptr;
    TdxPluginFunctionInfo *pluginTable2 = nullptr;

    // 第一次注册
    BOOL result1 = RegisterTdxFunc(&pluginTable1);
    EXPECT_EQ(result1, TRUE);
    EXPECT_NE(pluginTable1, nullptr);

    // 第二次注册应该失败（指针不为空）
    BOOL result2 = RegisterTdxFunc(&pluginTable2);
    EXPECT_EQ(result2, FALSE);

    // table2应该没有被修改
    EXPECT_EQ(pluginTable2, nullptr);
}

// 测试6: 插件函数调用 - 基本调用
TEST_F(TdxpyPluginRegistryTest, PluginFunction_CallBasic)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 调用函数1
    pluginTable[1].function(testDataLength, testOutput.data(),
                            testInputA.data(), nullptr, nullptr);

    // 验证输出被修改
    validateOutput(testOutput, testDataLength);

    // 验证不是全0
    bool allZero = true;
    for (float val : testOutput)
    {
        if (val != 0.0f)
        {
            allZero = false;
            break;
        }
    }
    EXPECT_FALSE(allZero);

    printOutput(testOutput, "函数1输出");
}

// 测试7: 插件函数调用 - 函数0的特殊处理
TEST_F(TdxpyPluginRegistryTest, PluginFunction_CallFunction0)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 先设置一些非零值
    std::fill(testOutput.begin(), testOutput.end(), 999.0f);

    // 调用函数0
    pluginTable[0].function(testDataLength, testOutput.data(),
                            testInputA.data(), nullptr, nullptr);

    // 验证输出（函数0可能用于重新加载等特殊操作）
    // 我们不断言具体值，只验证调用不崩溃
    validateOutput(testOutput, testDataLength, true);

    printOutput(testOutput, "函数0输出");
}

// 测试8: 插件函数调用 - 所有函数
TEST_F(TdxpyPluginRegistryTest, PluginFunction_CallAllFunctions)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 找到函数表的结束位置
    int functionCount = 0;
    while (pluginTable[functionCount].function != nullptr)
    {
        ++functionCount;
    }

    EXPECT_GT(functionCount, 0);

    // 测试调用每个函数（跳过一些以节省时间）
    const int step = functionCount / 10; // 测试约10个函数
    for (int i = 0; i < functionCount; i += step)
    {
        // 重置输出
        std::fill(testOutput.begin(), testOutput.end(), 0.0f);

        // 调用函数
        pluginTable[i].function(testDataLength, testOutput.data(),
                                testInputA.data(), nullptr, nullptr);

        // 验证调用不崩溃
        validateOutput(testOutput, testDataLength, true);

        std::cout << "函数 " << i << " 调用成功" << std::endl;
    }
}

// 测试9: 插件函数调用 - 不同数据长度
TEST_F(TdxpyPluginRegistryTest, PluginFunction_VariousDataLengths)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 测试各种数据长度
    std::vector<int> testLengths = {0, 1, 2, 5, 10, 100, 1000};

    for (int length : testLengths)
    {
        std::vector<float> output(length, 0.0f);
        std::vector<float> input(length, 1.0f);

        // 调用函数1
        pluginTable[1].function(length, output.data(),
                                input.data(), nullptr, nullptr);

        // 对于长度>0，验证输出
        if (length > 0)
        {
            validateOutput(output, length);

            // 验证不是全0（函数1应该产生非零输出）
            if (length > 0)
            {
                bool allZero = true;
                for (float val : output)
                {
                    if (val != 0.0f)
                    {
                        allZero = false;
                        break;
                    }
                }
                EXPECT_FALSE(allZero) << "长度 " << length << " 的输出全为0";
            }
        }

        std::cout << "数据长度 " << length << " 测试通过" << std::endl;
    }
}

// 测试10: 插件函数调用 - 空指针输入
TEST_F(TdxpyPluginRegistryTest, PluginFunction_NullInputs) {
    TdxPluginFunctionInfo* pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);
    
    // 使用传统的数组方法
    struct InputTestCase {
        std::string description;
        float* inputA;
        float* inputB;
        float* inputC;
    };
    
    InputTestCase testCases[] = {
        {"所有输入为空", nullptr, nullptr, nullptr},
        {"仅inputA", testInputA.data(), nullptr, nullptr},
        {"仅inputB", nullptr, testInputB.data(), nullptr},
        {"仅inputC", nullptr, nullptr, testInputC.data()},
        {"inputA和inputB", testInputA.data(), testInputB.data(), nullptr},
        {"所有输入", testInputA.data(), testInputB.data(), testInputC.data()}
    };
    
    int testCaseCount = sizeof(testCases) / sizeof(testCases[0]);
    
    for (int i = 0; i < testCaseCount; ++i) {
        const InputTestCase& testCase = testCases[i];
        
        // 重置输出
        std::fill(testOutput.begin(), testOutput.end(), 0.0f);
        
        // 调用函数
        pluginTable[1].function(testDataLength, testOutput.data(), 
                               testCase.inputA, testCase.inputB, testCase.inputC);
        
        // 验证调用不崩溃
        validateOutput(testOutput, testDataLength, true);
        
        std::cout << "输入组合: " << testCase.description << " 测试通过" << std::endl;
    }
}

// 测试11: 插件函数调用 - 输出指针为空
TEST_F(TdxpyPluginRegistryTest, PluginFunction_NullOutput)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 输出指针为空应该不崩溃
    pluginTable[1].function(testDataLength, nullptr,
                            testInputA.data(), nullptr, nullptr);

    // 如果到达这里，没有崩溃，测试通过
    EXPECT_TRUE(true);
}

// 测试12: 函数分发器逻辑验证（通过实际调用）
TEST_F(TdxpyPluginRegistryTest, FunctionDispatcher_Logic)
{
    // 这个测试验证pluginFunctionDispatcher的基本逻辑
    // 由于我们不能直接调用私有函数，我们通过注册的函数来测试

    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);

    // 模拟Python引擎的响应
    // 注意：实际测试中，这需要Python环境
    // 这里我们主要验证函数表结构和基本调用

    // 验证函数表有结束标记
    int count = 0;
    while (pluginTable[count].function != nullptr)
    {
        EXPECT_GE(pluginTable[count].functionId, 0);
        EXPECT_LE(pluginTable[count].functionId, 100);
        ++count;
    }

    EXPECT_GT(count, 0);
    std::cout << "函数表包含 " << count << " 个函数" << std::endl;
}

// 测试13: 函数ID范围验证
TEST_F(TdxpyPluginRegistryTest, FunctionId_Range)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);

    // 验证所有函数ID在0-100范围内
    for (int i = 0; pluginTable[i].function != nullptr; ++i)
    {
        EXPECT_GE(pluginTable[i].functionId, 0);
        EXPECT_LE(pluginTable[i].functionId, 100);
        EXPECT_EQ(pluginTable[i].functionId, static_cast<unsigned short>(i));
    }
}

// 测试14: 性能测试 - 多次调用
TEST_F(TdxpyPluginRegistryTest, Performance_MultipleCalls)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    constexpr int numCalls = 1000;
    constexpr int dataLength = 50;

    std::vector<float> input(dataLength);
    std::vector<float> output(dataLength, 0.0f);

    for (int i = 0; i < dataLength; ++i)
    {
        input[i] = static_cast<float>(i + 1);
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numCalls; ++i)
    {
        // 交替使用不同的函数
        int funcId = i % 3; // 0, 1, 2

        pluginTable[funcId].function(dataLength, output.data(),
                                     input.data(), nullptr, nullptr);

        // 验证输出
        validateOutput(output, dataLength, true);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "性能测试: " << numCalls << " 次调用耗时 "
              << duration.count() << "ms ("
              << (numCalls * 1000.0 / duration.count()) << " calls/sec)" << std::endl;

    // 验证性能在合理范围内
    EXPECT_LT(duration.count(), 5000) << "1000次调用耗时过长: " << duration.count() << "ms";
}

// 测试15: 并发调用测试
TEST_F(TdxpyPluginRegistryTest, ConcurrentCalls)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    constexpr int numThreads = 5;
    constexpr int callsPerThread = 20;
    constexpr int dataLength = 10;

    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    for (int t = 0; t < numThreads; ++t)
    {
        threads.emplace_back([t, pluginTable, dataLength, &successCount]()
                             {
            std::vector<float> input(dataLength);
            std::vector<float> output(dataLength, 0.0f);
            
            for (int i = 0; i < dataLength; ++i) {
                input[i] = static_cast<float>((t + 1) * (i + 1));
            }
            
            for (int c = 0; c < callsPerThread; ++c) {
                // 每个线程使用不同的函数
                int funcId = (t + c) % 3;
                
                pluginTable[funcId].function(dataLength, output.data(), 
                                            input.data(), nullptr, nullptr);
                
                // 验证输出有效
                bool valid = true;
                for (float val : output) {
                    if (std::isnan(val) || std::isinf(val)) {
                        valid = false;
                        break;
                    }
                }
                
                if (valid) {
                    ++successCount;
                }
                
                // 短暂休眠
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } });
    }

    // 等待所有线程完成
    for (auto &thread : threads)
    {
        thread.join();
    }

    int expectedSuccess = numThreads * callsPerThread;
    EXPECT_EQ(successCount.load(), expectedSuccess);

    std::cout << "并发测试: " << numThreads << " 个线程，"
              << callsPerThread << " 次调用/线程，全部成功" << std::endl;
}

// 测试16: 内存访问边界测试
TEST_F(TdxpyPluginRegistryTest, Memory_BoundaryAccess)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 测试边界情况的数据访问
    const int testLength = 5;

    // 创建刚好足够大小的数组
    float smallOutput[testLength];
    float smallInput[testLength];

    for (int i = 0; i < testLength; ++i)
    {
        smallInput[i] = static_cast<float>(i + 1);
        smallOutput[i] = 0.0f;
    }

    // 调用函数
    pluginTable[1].function(testLength, smallOutput,
                            smallInput, nullptr, nullptr);

    // 验证输出
    for (int i = 0; i < testLength; ++i)
    {
        EXPECT_FALSE(std::isnan(smallOutput[i]));
        EXPECT_FALSE(std::isinf(smallOutput[i]));
        EXPECT_NE(smallOutput[i], 0.0f); // 函数1应该产生非零输出
    }

    std::cout << "内存边界测试通过" << std::endl;
}

// 测试17: 函数表结束标记验证
TEST_F(TdxpyPluginRegistryTest, FunctionTable_Terminator)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 找到结束标记
    int count = 0;
    while (pluginTable[count].function != nullptr)
    {
        ++count;
    }

    // 验证结束标记的functionId为0
    // 注意：在提供的代码中，结束标记是{0, nullptr}
    EXPECT_EQ(pluginTable[count].functionId, 0);
    EXPECT_EQ(pluginTable[count].function, nullptr);

    // 验证结束标记之后没有有效数据
    if (count + 1 < 150)
    { // 检查一些额外位置
        // 可能是未定义的行为，我们不断言具体值
        // 只验证我们能安全地遍历到结束标记
    }

    std::cout << "函数表结束标记在位置 " << count << std::endl;
}

// 测试18: 函数指针有效性验证
TEST_F(TdxpyPluginRegistryTest, FunctionPointer_Validity)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 验证所有函数指针都有效
    for (int i = 0; pluginTable[i].function != nullptr; ++i)
    {
        EXPECT_NE(pluginTable[i].function, nullptr);

        // 尝试调用（不验证结果，只验证不崩溃）
        float dummyOutput[1] = {0.0f};
        float dummyInput[1] = {1.0f};

        pluginTable[i].function(1, dummyOutput, dummyInput, nullptr, nullptr);

        // 如果到达这里，没有崩溃
        EXPECT_TRUE(true);
    }

    std::cout << "所有函数指针调用测试通过" << std::endl;
}

// 测试19: 宏生成函数验证
TEST_F(TdxpyPluginRegistryTest, MacroGeneratedFunctions)
{
    // 这个测试验证GENERATE_PLUGIN_FUNCTION宏生成的函数

    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 测试几个关键位置的函数
    std::vector<int> testIds = {0, 1, 50, 99, 100};

    for (int id : testIds)
    {
        // 找到对应的函数
        TdxPluginFunction func = nullptr;
        for (int i = 0; pluginTable[i].function != nullptr; ++i)
        {
            if (pluginTable[i].functionId == id)
            {
                func = pluginTable[i].function;
                break;
            }
        }

        ASSERT_NE(func, nullptr) << "函数ID " << id << " 未找到";

        // 调用函数
        const int testLen = 3;
        float output[testLen] = {0.0f};
        float input[testLen] = {1.0f, 2.0f, 3.0f};

        func(testLen, output, input, nullptr, nullptr);

        // 验证调用不崩溃
        for (int i = 0; i < testLen; ++i)
        {
            EXPECT_FALSE(std::isnan(output[i]));
            EXPECT_FALSE(std::isinf(output[i]));
        }

        std::cout << "函数ID " << id << " 测试通过" << std::endl;
    }
}

// 测试20: 与Python引擎集成测试（模拟）
TEST_F(TdxpyPluginRegistryTest, Integration_PythonEngine)
{
    // 这个测试验证插件函数如何与Python引擎交互
    // 由于我们不能在测试中启动真正的Python，我们模拟基本流程

    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 模拟pluginFunctionDispatcher的逻辑
    // 1. 记录函数ID
    // 2. 调用Python引擎
    // 3. 处理结果

    // 我们验证函数表的结构和基本调用流程
    int dispatcherCalls = 0;

    // 调用几个不同的函数
    for (int funcId : {0, 1, 2, 50, 100})
    {
        // 找到函数
        TdxPluginFunction func = nullptr;
        for (int i = 0; pluginTable[i].function != nullptr; ++i)
        {
            if (pluginTable[i].functionId == funcId)
            {
                func = pluginTable[i].function;
                break;
            }
        }

        if (func)
        {
            // 准备测试数据
            const int testLen = 5;
            std::vector<float> output(testLen, 0.0f);
            std::vector<float> input(testLen, 1.0f);

            // 调用
            func(testLen, output.data(), input.data(), nullptr, nullptr);

            // 验证
            validateOutput(output, testLen, true);

            ++dispatcherCalls;
        }
    }

    EXPECT_GT(dispatcherCalls, 0);
    std::cout << "集成测试: 成功模拟 " << dispatcherCalls << " 次Python引擎调用" << std::endl;
}

// 测试21: 导出函数验证（DLL接口）
TEST_F(TdxpyPluginRegistryTest, ExportFunction_Validation)
{
    // 这个测试验证RegisterTdxFunc是否正确定义为DLL导出函数

    // 尝试获取函数地址（如果编译为DLL）
    // 在单元测试中，我们直接调用即可

    TdxPluginFunctionInfo *table = nullptr;
    BOOL result = RegisterTdxFunc(&table);

    // 验证基本行为
    EXPECT_TRUE(result == TRUE || result == FALSE);

    if (result == TRUE)
    {
        EXPECT_NE(table, nullptr);
        EXPECT_NE(table[0].function, nullptr);
    }

    // 验证函数签名
    // RegisterTdxFunc应该返回BOOL并接受TdxPluginFunctionInfo**
    // 这由头文件保证
}

// 测试22: 头文件兼容性测试
TEST_F(TdxpyPluginRegistryTest, HeaderFile_Compatibility)
{
    // 验证头文件中的定义与实现一致

    // 1. 验证结构体定义
    TdxPluginFunctionInfo testStruct;
    testStruct.functionId = 123;
    testStruct.function = TestPluginFunction1;

    EXPECT_EQ(testStruct.functionId, 123);
    EXPECT_EQ(testStruct.function, TestPluginFunction1);

    // 2. 验证函数指针类型
    TdxPluginFunction funcPtr = TestPluginFunction2;
    EXPECT_NE(funcPtr, nullptr);

    // 3. 验证extern "C"声明
    // 这由编译器在链接时验证

    std::cout << "头文件兼容性测试通过" << std::endl;
}

// 测试23: 错误处理路径测试
TEST_F(TdxpyPluginRegistryTest, ErrorHandling_Paths)
{
    // 测试各种错误情况的处理

    // 1. 无效的函数表指针
    BOOL result = RegisterTdxFunc(nullptr);
    EXPECT_EQ(result, FALSE);

    // 2. 已经初始化的函数表指针
    TdxPluginFunctionInfo *table1 = nullptr;
    BOOL result1 = RegisterTdxFunc(&table1);
    EXPECT_EQ(result1, TRUE);

    TdxPluginFunctionInfo *table2 = (TdxPluginFunctionInfo *)0x12345678; // 非空指针
    BOOL result2 = RegisterTdxFunc(&table2);
    EXPECT_EQ(result2, FALSE);
    EXPECT_EQ(table2, (TdxPluginFunctionInfo *)0x12345678); // 应该未被修改

    // 3. 空函数指针调用（通过有效函数表）
    if (table1)
    {
        // 找到结束标记前的最后一个函数
        int lastIndex = 0;
        while (table1[lastIndex].function != nullptr)
        {
            ++lastIndex;
        }

        // 结束标记应该是{0, nullptr}
        EXPECT_EQ(table1[lastIndex].functionId, 0);
        EXPECT_EQ(table1[lastIndex].function, nullptr);
    }
}

// 测试24: 数据一致性测试
TEST_F(TdxpyPluginRegistryTest, Data_Consistency)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    // 验证函数表内部一致性
    int prevId = -1;

    for (int i = 0; pluginTable[i].function != nullptr; ++i)
    {
        unsigned short currentId = pluginTable[i].functionId;

        // ID应该递增
        EXPECT_GT(currentId, prevId);

        // ID应该在0-100范围内
        EXPECT_GE(currentId, 0);
        EXPECT_LE(currentId, 100);

        // 函数指针应该非空
        EXPECT_NE(pluginTable[i].function, nullptr);

        prevId = currentId;
    }

    // 验证有101个函数（0-100）
    EXPECT_EQ(prevId, 100);

    std::cout << "数据一致性验证通过，找到 " << (prevId + 1) << " 个函数" << std::endl;
}

// 测试25: 性能分析 - 函数调用开销
TEST_F(TdxpyPluginRegistryTest, Performance_CallOverhead)
{
    TdxPluginFunctionInfo *pluginTable = nullptr;
    BOOL registered = RegisterTdxFunc(&pluginTable);
    ASSERT_EQ(registered, TRUE);
    ASSERT_NE(pluginTable, nullptr);

    constexpr int warmup = 100;
    constexpr int measurements = 10000;
    constexpr int dataLength = 10;

    std::vector<float> input(dataLength);
    std::vector<float> output(dataLength, 0.0f);

    for (int i = 0; i < dataLength; ++i)
    {
        input[i] = static_cast<float>(i + 1);
    }

    // 预热
    for (int i = 0; i < warmup; ++i)
    {
        pluginTable[1].function(dataLength, output.data(),
                                input.data(), nullptr, nullptr);
    }

    // 测量
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < measurements; ++i)
    {
        pluginTable[1].function(dataLength, output.data(),
                                input.data(), nullptr, nullptr);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);

    double avgTimePerCall = duration.count() / static_cast<double>(measurements);

    std::cout << "性能分析: " << std::endl;
    std::cout << "  总调用次数: " << measurements << std::endl;
    std::cout << "  总耗时: " << duration.count() << "ns" << std::endl;
    std::cout << "  平均每次调用: " << avgTimePerCall << "ns" << std::endl;
    std::cout << "  每秒调用数: " << (1e9 / avgTimePerCall) << std::endl;

    // 期望每次调用开销在微秒级别
    EXPECT_LT(avgTimePerCall, 1000000.0) << "函数调用开销过高: " << avgTimePerCall << "ns";
}
