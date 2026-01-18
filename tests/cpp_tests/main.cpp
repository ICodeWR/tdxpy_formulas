// tests/main.cpp - 带调试信息的Google Test入口
/**
 * @file        main.cpp
 * @brief       单元测试主程序
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

#include "gtest/gtest.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char **argv) {
    std::cout << u8"=== 测试程序开始 ===" << std::endl;
    
#ifdef _WIN32
    // 确保控制台使用UTF-8
    SetConsoleOutputCP(CP_UTF8);
    std::cout << u8"控制台编码已设置为UTF-8" << std::endl;
#endif
    
    std::cout << u8"正在初始化Google Test..." << std::endl;
    
    // 初始化Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << u8"Google Test初始化完成" << std::endl;
    std::cout << u8"开始运行测试..." << std::endl;
    std::cout << u8"========================" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << u8"========================" << std::endl;
    std::cout << u8"测试运行完成，返回值: " << result << std::endl;
    
    // Windows下暂停，以便查看输出
#ifdef _WIN32
    system("pause");
#endif
    
    return result;
}
