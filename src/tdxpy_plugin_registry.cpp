/**
 * @file        tdxpy_plugin_registry.cpp
 * @brief       通达信DLL插件函数实现与注册
 * @author      码上工坊
 * @copyright   Copyright (c) 2026-2030 码上工坊 Contributors
 * @license     MIT License (详见项目根目录LICENSE文件)
 * @version     0.1.0
 * @date        2026-01-05
 *
 * @par 修改记录:
 * <table>
 * <tr><th>日期         <th>版本      <th>作者              <th>描述
 * <tr><td>2026-01-05   <td>0.1.0    <td>码上工坊           <td>初始版本
 * </table>
 */

#include "pch.h"
#include "tdxpy_plugin_registry.h"
#include "tdxpy_python_engine.h"
#include "tdxpy_logger.h"

namespace
{

    /**
     * @brief 通用插件函数调度器
     * @param functionId 功能ID号
     * @param dataLength 数据长度
     * @param output 输出数据数组
     * @param inputA 第一组输入数据数组
     * @param inputB 第二组输入数据数组
     * @param inputC 第三组输入数据数组
     */
    void pluginFunctionDispatcher(int functionId, int dataLength, float *output,
                                  float *inputA, float *inputB, float *inputC)
    {
        TDXPY_LOG_DEBUG(u8"通用插件函数调度器: " + std::to_string(functionId) + u8", 数据长度: " + std::to_string(dataLength));

        if (!tdxpyRunPythonPlugin(functionId, dataLength, output, inputA, inputB, inputC))
        {
            // 第 0 号函数特殊处理：重新加载 Python 模块
            if (functionId)
            {
                for (int i = 0; i < dataLength; i++)
                {
                    output[i] = 0;
                }
            }
        }
    }

    // 生成101个插件函数
#define GENERATE_PLUGIN_FUNCTION(n)                                                                    \
    void pluginFunction##n(int dataLength, float *output, float *inputA, float *inputB, float *inputC) \
    {                                                                                                  \
        pluginFunctionDispatcher(n, dataLength, output, inputA, inputB, inputC);                       \
    }

    // 生成0-100号插件函数
    GENERATE_PLUGIN_FUNCTION(0)
    GENERATE_PLUGIN_FUNCTION(1)
    GENERATE_PLUGIN_FUNCTION(2)
    GENERATE_PLUGIN_FUNCTION(3)
    GENERATE_PLUGIN_FUNCTION(4)
    GENERATE_PLUGIN_FUNCTION(5)
    GENERATE_PLUGIN_FUNCTION(6)
    GENERATE_PLUGIN_FUNCTION(7)
    GENERATE_PLUGIN_FUNCTION(8)
    GENERATE_PLUGIN_FUNCTION(9)
    GENERATE_PLUGIN_FUNCTION(10)
    GENERATE_PLUGIN_FUNCTION(11)
    GENERATE_PLUGIN_FUNCTION(12)
    GENERATE_PLUGIN_FUNCTION(13)
    GENERATE_PLUGIN_FUNCTION(14)
    GENERATE_PLUGIN_FUNCTION(15)
    GENERATE_PLUGIN_FUNCTION(16)
    GENERATE_PLUGIN_FUNCTION(17)
    GENERATE_PLUGIN_FUNCTION(18)
    GENERATE_PLUGIN_FUNCTION(19)
    GENERATE_PLUGIN_FUNCTION(20)
    GENERATE_PLUGIN_FUNCTION(21)
    GENERATE_PLUGIN_FUNCTION(22)
    GENERATE_PLUGIN_FUNCTION(23)
    GENERATE_PLUGIN_FUNCTION(24)
    GENERATE_PLUGIN_FUNCTION(25)
    GENERATE_PLUGIN_FUNCTION(26)
    GENERATE_PLUGIN_FUNCTION(27)
    GENERATE_PLUGIN_FUNCTION(28)
    GENERATE_PLUGIN_FUNCTION(29)
    GENERATE_PLUGIN_FUNCTION(30)
    GENERATE_PLUGIN_FUNCTION(31)
    GENERATE_PLUGIN_FUNCTION(32)
    GENERATE_PLUGIN_FUNCTION(33)
    GENERATE_PLUGIN_FUNCTION(34)
    GENERATE_PLUGIN_FUNCTION(35)
    GENERATE_PLUGIN_FUNCTION(36)
    GENERATE_PLUGIN_FUNCTION(37)
    GENERATE_PLUGIN_FUNCTION(38)
    GENERATE_PLUGIN_FUNCTION(39)
    GENERATE_PLUGIN_FUNCTION(40)
    GENERATE_PLUGIN_FUNCTION(41)
    GENERATE_PLUGIN_FUNCTION(42)
    GENERATE_PLUGIN_FUNCTION(43)
    GENERATE_PLUGIN_FUNCTION(44)
    GENERATE_PLUGIN_FUNCTION(45)
    GENERATE_PLUGIN_FUNCTION(46)
    GENERATE_PLUGIN_FUNCTION(47)
    GENERATE_PLUGIN_FUNCTION(48)
    GENERATE_PLUGIN_FUNCTION(49)
    GENERATE_PLUGIN_FUNCTION(50)
    GENERATE_PLUGIN_FUNCTION(51)
    GENERATE_PLUGIN_FUNCTION(52)
    GENERATE_PLUGIN_FUNCTION(53)
    GENERATE_PLUGIN_FUNCTION(54)
    GENERATE_PLUGIN_FUNCTION(55)
    GENERATE_PLUGIN_FUNCTION(56)
    GENERATE_PLUGIN_FUNCTION(57)
    GENERATE_PLUGIN_FUNCTION(58)
    GENERATE_PLUGIN_FUNCTION(59)
    GENERATE_PLUGIN_FUNCTION(60)
    GENERATE_PLUGIN_FUNCTION(61)
    GENERATE_PLUGIN_FUNCTION(62)
    GENERATE_PLUGIN_FUNCTION(63)
    GENERATE_PLUGIN_FUNCTION(64)
    GENERATE_PLUGIN_FUNCTION(65)
    GENERATE_PLUGIN_FUNCTION(66)
    GENERATE_PLUGIN_FUNCTION(67)
    GENERATE_PLUGIN_FUNCTION(68)
    GENERATE_PLUGIN_FUNCTION(69)
    GENERATE_PLUGIN_FUNCTION(70)
    GENERATE_PLUGIN_FUNCTION(71)
    GENERATE_PLUGIN_FUNCTION(72)
    GENERATE_PLUGIN_FUNCTION(73)
    GENERATE_PLUGIN_FUNCTION(74)
    GENERATE_PLUGIN_FUNCTION(75)
    GENERATE_PLUGIN_FUNCTION(76)
    GENERATE_PLUGIN_FUNCTION(77)
    GENERATE_PLUGIN_FUNCTION(78)
    GENERATE_PLUGIN_FUNCTION(79)
    GENERATE_PLUGIN_FUNCTION(80)
    GENERATE_PLUGIN_FUNCTION(81)
    GENERATE_PLUGIN_FUNCTION(82)
    GENERATE_PLUGIN_FUNCTION(83)
    GENERATE_PLUGIN_FUNCTION(84)
    GENERATE_PLUGIN_FUNCTION(85)
    GENERATE_PLUGIN_FUNCTION(86)
    GENERATE_PLUGIN_FUNCTION(87)
    GENERATE_PLUGIN_FUNCTION(88)
    GENERATE_PLUGIN_FUNCTION(89)
    GENERATE_PLUGIN_FUNCTION(90)
    GENERATE_PLUGIN_FUNCTION(91)
    GENERATE_PLUGIN_FUNCTION(92)
    GENERATE_PLUGIN_FUNCTION(93)
    GENERATE_PLUGIN_FUNCTION(94)
    GENERATE_PLUGIN_FUNCTION(95)
    GENERATE_PLUGIN_FUNCTION(96)
    GENERATE_PLUGIN_FUNCTION(97)
    GENERATE_PLUGIN_FUNCTION(98)
    GENERATE_PLUGIN_FUNCTION(99)
    GENERATE_PLUGIN_FUNCTION(100)

#undef GENERATE_PLUGIN_FUNCTION

    // 插件函数表
    static TdxPluginFunctionInfo g_pluginFunctionTable[] = {
        {0, pluginFunction0},
        {1, pluginFunction1},
        {2, pluginFunction2},
        {3, pluginFunction3},
        {4, pluginFunction4},
        {5, pluginFunction5},
        {6, pluginFunction6},
        {7, pluginFunction7},
        {8, pluginFunction8},
        {9, pluginFunction9},
        {10, pluginFunction10},
        {11, pluginFunction11},
        {12, pluginFunction12},
        {13, pluginFunction13},
        {14, pluginFunction14},
        {15, pluginFunction15},
        {16, pluginFunction16},
        {17, pluginFunction17},
        {18, pluginFunction18},
        {19, pluginFunction19},
        {20, pluginFunction20},
        {21, pluginFunction21},
        {22, pluginFunction22},
        {23, pluginFunction23},
        {24, pluginFunction24},
        {25, pluginFunction25},
        {26, pluginFunction26},
        {27, pluginFunction27},
        {28, pluginFunction28},
        {29, pluginFunction29},
        {30, pluginFunction30},
        {31, pluginFunction31},
        {32, pluginFunction32},
        {33, pluginFunction33},
        {34, pluginFunction34},
        {35, pluginFunction35},
        {36, pluginFunction36},
        {37, pluginFunction37},
        {38, pluginFunction38},
        {39, pluginFunction39},
        {40, pluginFunction40},
        {41, pluginFunction41},
        {42, pluginFunction42},
        {43, pluginFunction43},
        {44, pluginFunction44},
        {45, pluginFunction45},
        {46, pluginFunction46},
        {47, pluginFunction47},
        {48, pluginFunction48},
        {49, pluginFunction49},
        {50, pluginFunction50},
        {51, pluginFunction51},
        {52, pluginFunction52},
        {53, pluginFunction53},
        {54, pluginFunction54},
        {55, pluginFunction55},
        {56, pluginFunction56},
        {57, pluginFunction57},
        {58, pluginFunction58},
        {59, pluginFunction59},
        {60, pluginFunction60},
        {61, pluginFunction61},
        {62, pluginFunction62},
        {63, pluginFunction63},
        {64, pluginFunction64},
        {65, pluginFunction65},
        {66, pluginFunction66},
        {67, pluginFunction67},
        {68, pluginFunction68},
        {69, pluginFunction69},
        {70, pluginFunction70},
        {71, pluginFunction71},
        {72, pluginFunction72},
        {73, pluginFunction73},
        {74, pluginFunction74},
        {75, pluginFunction75},
        {76, pluginFunction76},
        {77, pluginFunction77},
        {78, pluginFunction78},
        {79, pluginFunction79},
        {80, pluginFunction80},
        {81, pluginFunction81},
        {82, pluginFunction82},
        {83, pluginFunction83},
        {84, pluginFunction84},
        {85, pluginFunction85},
        {86, pluginFunction86},
        {87, pluginFunction87},
        {88, pluginFunction88},
        {89, pluginFunction89},
        {90, pluginFunction90},
        {91, pluginFunction91},
        {92, pluginFunction92},
        {93, pluginFunction93},
        {94, pluginFunction94},
        {95, pluginFunction95},
        {96, pluginFunction96},
        {97, pluginFunction97},
        {98, pluginFunction98},
        {99, pluginFunction99},
        {100, pluginFunction100},
        {0, nullptr} // 结束标记
    };

} // namespace

/**
 * @brief 注册通达信插件函数
 * @param pluginFuncTable 插件函数表的二级指针
 * @return 注册成功返回TRUE，失败返回FALSE
 */
BOOL RegisterTdxFunc(TdxPluginFunctionInfo **pluginFuncTable)
{
    TDXPY_LOG_DEBUG(u8"注册通达信插件函数: " + std::to_string(sizeof(g_pluginFunctionTable) / sizeof(TdxPluginFunctionInfo) - 1) + u8" 个函数");

    if (*pluginFuncTable == nullptr)
    {
        (*pluginFuncTable) = g_pluginFunctionTable;
        return TRUE;
    }

    return FALSE;
}