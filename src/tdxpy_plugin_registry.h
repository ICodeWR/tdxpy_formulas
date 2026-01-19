/**
 * @file        tdxpy_plugin_registry.h
 * @brief       通达信DLL插件注册函数和类型定义头文件
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

#ifndef TDXPY_PLUGIN_REGISTRY_H
#define TDXPY_PLUGIN_REGISTRY_H

#pragma pack(push, 1)

/// 插件函数类型定义 (数据个数, 输出, 输入a, 输入b, 输入c)
typedef void (*TdxPluginFunction)(int dataLength, float *output,
                                  float *inputA, float *inputB, float *inputC);

/// 插件函数信息结构体
typedef struct TdxPluginFunctionInfo
{
    unsigned short functionId;  ///< 功能ID号，即调用第几号回调函数
    TdxPluginFunction function; ///< 回调函数指针
} TdxPluginFunctionInfo;

#pragma pack(pop)

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /**
     * @brief 注册通达信插件函数
     * @param pluginFuncTable 插件函数表的二级指针
     * @return 注册成功返回TRUE，失败返回FALSE
     * @retval TRUE 成功注册插件函数表
     * @retval FALSE 插件函数表已存在或注册失败
     */
    extern "C" __declspec(dllexport) BOOL RegisterTdxFunc(TdxPluginFunctionInfo **pluginFuncTable);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TDXPY_PLUGIN_REGISTRY_H
