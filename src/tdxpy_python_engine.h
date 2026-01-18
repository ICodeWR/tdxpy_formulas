/**
 * @file        tdxpy_python_engine.h
 * @brief       C++调用Python引擎头文件
 * @author      码上工坊
 * @copyright   Copyright (c) 2026-2030 码上工坊 Contributors
 * @license     MIT License (详见项目根目录LICENSE文件)
 * @version     0.1.0
 * @date        2026-01-05
 *
 * @par 修改记录:
 * <table>
 * <tr><th>日期         <th>版本      <th>作者              <th>描述
 * <tr><td>2026-01-05  <td>0.1.0    <td>码上工坊          <td>初始版本
 * </table>
 */

#ifndef TDXPY_PYTHON_ENGINE_H
#define TDXPY_PYTHON_ENGINE_H

#include <iostream>
#include <fstream>
#include <string>

/// 模块重新加载开关，1:每次调用重新导入, 0:缓存模块
#define TDXPY_MODULE_RELOAD_ENABLED 1

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /**
     * @brief 初始化Python引擎
     * @return 成功返回1，失败返回0
     */
    int tdxpyPythonInitialize(void);

    /**
     * @brief 释放Python引擎资源
     * @return 始终返回1
     */
    int tdxpyPythonDeinitialize(void);

    /**
     * @brief 运行Python插件函数
     * @param functionId 功能ID号
     * @param dataLength 数据长度
     * @param output 输出数据数组
     * @param inputA 第一组输入数据数组
     * @param inputB 第二组输入数据数组
     * @param inputC 第三组输入数据数组
     * @return 成功返回1，失败返回0
     */
    int tdxpyRunPythonPlugin(int functionId, int dataLength,
                             float *output,
                             float *inputA, float *inputB, float *inputC);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !TDXPY_PYTHON_ENGINE_H