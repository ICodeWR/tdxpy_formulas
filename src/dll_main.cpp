/**
 * @file        dll_main.cpp
 * @brief       通达信插件DLL加载入口点
 * @author      码上工坊
 * @copyright   Copyright (c) 2026-2030 码上工坊 Contributors
 * @license     MIT License (详见项目根目录LICENSE文件)
 * @version     0.1.0
 * @date        2026-01-08
 *
 * @par 修改记录:
 * <table>
 * <tr><th>日期         <th>版本      <th>作者              <th>描述
 * <tr><td>2026-01-08   <td>0.1.0    <td>码上工坊          <td>初始版本
 * </table>
 */

#include "pch.h"
#include "tdxpy_python_engine.h"

// The following symbol used to force inclusion of this module for _USRDLL

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    // 防止未使用参数警告
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        tdxpyPythonInitialize();  // 初始化Python引擎
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        // 不需要特殊处理
        break;

    case DLL_PROCESS_DETACH:
        tdxpyPythonDeinitialize(); // 释放Python引擎资源
        break;
    }
    
    return TRUE;
}
