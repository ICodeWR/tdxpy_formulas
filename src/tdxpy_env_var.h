/**
 * @file        tdxpy_env_var.h
 * @brief       通达信Python DLL环境变量管理器
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

#pragma once

#include <string>
#include <optional>
#include <vector>
#include <windows.h>

namespace tdxpy
{
    namespace utils
    {

        /**
         * @brief 环境变量管理器（纯工具类）
         * @details 提供跨平台的环境变量安全操作，支持UTF-8
         */
        class EnvVarManager final
        {
        public:
            // 禁止实例化
            EnvVarManager() = delete;
            ~EnvVarManager() = delete;

            // 禁止拷贝和移动
            EnvVarManager(const EnvVarManager &) = delete;
            EnvVarManager &operator=(const EnvVarManager &) = delete;
            EnvVarManager(EnvVarManager &&) = delete;
            EnvVarManager &operator=(EnvVarManager &&) = delete;

            // === 基本操作 ===

            /**
             * @brief 获取环境变量（UTF-8）
             */
            static std::optional<std::string> get(const std::string &name);

            /**
             * @brief 获取环境变量（宽字符）
             */
            static std::optional<std::wstring> getW(const std::wstring &name);

            /**
             * @brief 设置环境变量（UTF-8）
             */
            static bool set(const std::string &name, const std::string &value);

            /**
             * @brief 设置环境变量（宽字符）
             */
            static bool setW(const std::wstring &name, const std::wstring &value);

            /**
             * @brief 删除环境变量
             */
            static bool remove(const std::string &name);

            /**
             * @brief 检查环境变量是否存在
             */
            static bool exists(const std::string &name);

            // === 批量操作 ===

            /**
             * @brief 批量获取环境变量
             */
            static std::vector<std::pair<std::string, std::string>> getAll();

            /**
             * @brief 备份当前环境变量
             */
            static std::vector<std::pair<std::string, std::string>> backup();

            /**
             * @brief 从备份恢复环境变量
             */
            static bool restore(const std::vector<std::pair<std::string, std::string>> &backup);

            // === 路径操作 ===

            /**
             * @brief 在PATH中添加路径
             */
            static bool addToPath(const std::string &path, bool front = true);

            /**
             * @brief 从PATH中移除路径
             */
            static bool removeFromPath(const std::string &path);

            /**
             * @brief 检查路径是否在PATH中
             */
            static bool isInPath(const std::string &path);

            // === 编码转换 ===

            /**
             * @brief UTF-8转宽字符串
             */
            static std::optional<std::wstring> utf8ToWide(const std::string &utf8Str);

            /**
             * @brief 宽字符串转UTF-8
             */
            static std::optional<std::string> wideToUtf8(const std::wstring &wideStr);

        private:
            /**
             * @brief 安全的获取环境变量（内部实现）
             */
            static DWORD safeGetEnvVar(const wchar_t *name, wchar_t *buffer, DWORD bufferSize);
            /**
             * @brief 分割PATH字符串
             */
            static std::vector<std::string> splitPath(const std::string &pathStr);

            /**
             * @brief 合并PATH字符串
             */
            static std::string joinPath(const std::vector<std::string> &paths);
        };

    } // namespace utils
} // namespace tdxpy