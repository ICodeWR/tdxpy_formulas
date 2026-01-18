// mock_tdxpy_python_engine.h
/**
 * @file        mock_tdxpy_python_engine.h
 * @brief       模拟Python引擎接口声明
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
#ifndef MOCK_TDXPY_PYTHON_ENGINE_H
#define MOCK_TDXPY_PYTHON_ENGINE_H

#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

// Python引擎初始化
int mock_tdxpyPythonInitialize();

// Python引擎反初始化
int mock_tdxpyPythonDeinitialize();

// 检查Python是否已初始化
int tdxpyIsPythonInitialized();

// 获取Python版本
const char* tdxpyGetPythonVersion();

// 运行Python插件
int mock_tdxpyRunPythonPlugin(int functionId, int dataLength, float* output,
                        float* inputA, float* inputB, float* inputC);

// 获取最后使用的函数ID
int tdxpyGetLastFunctionId();

// 重新加载配置
int tdxpyReloadConfig();

// 检查是否是Python 3.14或更高版本
int tdxpyIsPython314OrHigher();

#ifdef __cplusplus
}
#endif

#endif // MOCK_TDXPY_PYTHON_ENGINE_H