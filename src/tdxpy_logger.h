/**
 * @file        tdxpy_logger.h
 * @brief       通达信Python DLL日志管理
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
#ifndef TDXPY_LOGGER_H
#define TDXPY_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>

namespace tdxpy
{

    /**
     * @brief 日志级别枚举
     */
    enum LogLevel
    {
        LogTrace = 1,   ///< 详细级别
        LogDebug = 2,   ///< 调试级别
        LogInfo = 3,    ///< 信息级别
        LogWarning = 4, ///< 警告级别
        LogError = 5,   ///< 错误级别
        LogOff = 6      ///< 关闭日志
    };

    class Logger
    {
    public:
        /**
         * @brief 获取单例实例
         */
        static Logger &getInstance()
        {
            static Logger instance;
            return instance;
        }

        /**
         * @brief 初始化日志系统
         * @param logFilePath 日志文件路径
         * @param minLevel 最小日志级别
         * @return 初始化成功返回true，失败返回false
         */
        bool initialize(const std::string &logFilePath, LogLevel minLevel = LogInfo);

        /**
         * @brief 设置最小日志级别
         * @param level 日志级别
         */
        void setMinLevel(LogLevel level);

        /**
         * @brief 日志输出函数
         * @param level 日志级别
         * @param function 函数名
         * @param file 文件名
         * @param line 行号
         * @param message 日志消息
         */
        void log(LogLevel level, const std::string &function, const std::string &file,
                 int line, const std::string &message);

        /**
         * @brief 详细调试级别日志（目前和debug没有严格区别，属于预留接口）
         */
        void trace(const std::string &function, const std::string &file,
                   int line, const std::string &message);

        /**
         * @brief 调试级别日志
         */
        void debug(const std::string &function, const std::string &file,
                   int line, const std::string &message);

        /**
         * @brief 信息级别日志
         */
        void info(const std::string &function, const std::string &file,
                  int line, const std::string &message);

        /**
         * @brief 警告级别日志
         */
        void warning(const std::string &function, const std::string &file,
                     int line, const std::string &message);

        /**
         * @brief 错误级别日志
         */
        void error(const std::string &function, const std::string &file,
                   int line, const std::string &message);

        /**
         * @brief 清理资源
         */
        void cleanup();

        LogLevel getLevelFromString(const std::string &levelStr);

    private:
        Logger();
        ~Logger();
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;

        std::string getCurrentTime();
        std::string levelToString(LogLevel level);
        std::string extractFileName(const std::string &fullPath);

        std::ofstream m_logFile; ///< 日志文件流
        std::mutex m_logMutex;   ///< 日志互斥锁
        LogLevel m_minLevel;     ///< 最小日志级别
        bool m_isInitialized;    ///< 是否已初始化
    };

    // 宏定义，简化调用
#define TDXPY_LOG_TRACE(msg) tdxpy::Logger::getInstance().trace(__FUNCTION__, __FILE__, __LINE__, msg)
#define TDXPY_LOG_DEBUG(msg) tdxpy::Logger::getInstance().debug(__FUNCTION__, __FILE__, __LINE__, msg)
#define TDXPY_LOG_INFO(msg) tdxpy::Logger::getInstance().info(__FUNCTION__, __FILE__, __LINE__, msg)
#define TDXPY_LOG_WARNING(msg) tdxpy::Logger::getInstance().warning(__FUNCTION__, __FILE__, __LINE__, msg)
#define TDXPY_LOG_ERROR(msg) tdxpy::Logger::getInstance().error(__FUNCTION__, __FILE__, __LINE__, msg)

} // namespace tdxpy

#endif // TDXPY_LOGGER_H