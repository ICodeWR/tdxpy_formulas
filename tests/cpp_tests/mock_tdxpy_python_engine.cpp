// mock_tdxpy_python_engine.cpp
/**
 * @file        mock_tdxpy_python_engine.cpp
 * @brief       模拟Python引擎接口实现
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
#include "mock_tdxpy_python_engine.h"
#include <vector>
#include <iostream>

// 全局状态
static int g_lastFunctionId = 0;
static bool g_pythonInitialized = false;

int mock_tdxpyPythonInitialize() { 
    std::cout << "模拟: mock_tdxpyPythonInitialize()" << std::endl;
    g_pythonInitialized = true;
    return 0;
}

int mock_tdxpyPythonDeinitialize() { 
    std::cout << "模拟: mock_tdxpyPythonDeinitialize()" << std::endl;
    g_pythonInitialized = false;
    return 1;
}

int tdxpyIsPythonInitialized() { 
    return g_pythonInitialized ? 1 : 0;
}

const char* tdxpyGetPythonVersion() { 
    return "3.14.2";
}

int mock_tdxpyRunPythonPlugin(int functionId, int dataLength, float* output,
                        float* inputA, float* inputB, float* inputC) {
    g_lastFunctionId = functionId;
    
    std::cout << "模拟: tdxpyRunPythonPlugin(functionId=" << functionId 
              << ", dataLength=" << dataLength << ")" << std::endl;
    
    if (!output || dataLength <= 0) {
        return 0;
    }
    
    // 模拟一些数据处理
    float base = 1.0f;
    if (inputA && dataLength > 0) {
        base = inputA[0];
    }
    
    for (int i = 0; i < dataLength; ++i) {
        output[i] = base * (i + 1) * 0.5f;
    }
    
    return 1;
}

int tdxpyGetLastFunctionId() {
    return g_lastFunctionId;
}

int tdxpyReloadConfig() {
    return 1;
}

int tdxpyIsPython314OrHigher() {
    return 1;
}