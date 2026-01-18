/**
 * @file        tdxpy_logger.cpp
 * @brief       通达信Python DLL日志管理实现
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


 // 如果需要禁用特定安全警告
#define _CRT_SECURE_NO_WARNINGS

#include <iomanip>
#include <sstream>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <windows.h>
#include "tdxpy_logger.h"

namespace tdxpy
{

    Logger::Logger() : m_minLevel(LogInfo), m_isInitialized(false)
    {
    }

    Logger::~Logger()
    {
        cleanup();
    }

    bool Logger::initialize(const std::string& logFilePath, LogLevel minLevel)
    {
        std::lock_guard<std::mutex> lock(m_logMutex);

        if (m_isInitialized)
        {
            m_logFile.close();
        }

        SetConsoleOutputCP(CP_UTF8);

        m_logFile.open(logFilePath, std::ios::app);
        if (!m_logFile.is_open())
        {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            m_isInitialized = false;
            return false;
        }

        m_minLevel = minLevel;
        m_isInitialized = true;

        // 写入初始化信息
        m_logFile << "=== Logging started at " << getCurrentTime() << " ===" << std::endl;
        m_logFile.flush();

        return true;
    }

    void Logger::setMinLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_minLevel = level;
    }

    void Logger::log(LogLevel level, const std::string& function, const std::string& file,
        int line, const std::string& message)
    {
        if (level < m_minLevel)
            return;

        std::lock_guard<std::mutex> lock(m_logMutex);

        // 提取文件名（去掉路径）
        std::string fileName = extractFileName(file);

        // 构建日志行，包含文件行号
        std::string logLine = getCurrentTime() + " [" + levelToString(level) + "] " +
            fileName + ":" + std::to_string(line) + " " +
            function + "() - " + message;

        // 输出到控制台
        std::cout << logLine << std::endl;

        // 输出到文件
        if (m_isInitialized && m_logFile.is_open())
        {
            m_logFile << logLine << std::endl;
            m_logFile.flush();
        }
    }

    void Logger::trace(const std::string& function, const std::string& file,
        int line, const std::string& message)
    {
        log(LogTrace, function, file, line, message);
    }

    void Logger::debug(const std::string& function, const std::string& file,
        int line, const std::string& message)
    {
        log(LogDebug, function, file, line, message);
    }

    void Logger::info(const std::string& function, const std::string& file,
        int line, const std::string& message)
    {
        log(LogInfo, function, file, line, message);
    }

    void Logger::warning(const std::string& function, const std::string& file,
        int line, const std::string& message)
    {
        log(LogWarning, function, file, line, message);
    }

    void Logger::error(const std::string& function, const std::string& file,
        int line, const std::string& message)
    {
        log(LogError, function, file, line, message);
    }

    void Logger::cleanup()
    {
        std::lock_guard<std::mutex> lock(m_logMutex);

        if (m_isInitialized && m_logFile.is_open())
        {
            m_logFile << "=== Logging stopped at " << getCurrentTime() << " ===" << std::endl;
            m_logFile.close();
        }

        m_isInitialized = false;
    }

    std::string Logger::extractFileName(const std::string& fullPath)
    {
        // 找到最后一个路径分隔符
#if defined(_WIN32) || defined(_WIN64)
        size_t pos = fullPath.find_last_of("\\/");
#else
        size_t pos = fullPath.find_last_of('/');
#endif

        if (pos != std::string::npos)
        {
            return fullPath.substr(pos + 1);
        }
        return fullPath;
    }

    std::string Logger::getCurrentTime()
    {
        // 使用C++11的chrono（推荐，跨平台安全）
        auto now = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

        // 获取毫秒
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) %
            1000;

        // 安全版本的 localtime（使用本地时间）
        std::tm localTm;

#if defined(_MSC_VER) && _MSC_VER >= 1400 // Visual Studio 2005+
        // Windows 安全版本
        localtime_s(&localTm, &nowTime);
#else
        // 其他平台的兼容版本
        std::tm* tmp = std::localtime(&nowTime);
        if (tmp)
        {
            localTm = *tmp;
        }
        else
        {
            // 如果失败，返回错误信息
            return "Time Error";
        }
#endif

        std::ostringstream oss;
        oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    std::string Logger::levelToString(LogLevel level)
    {
        switch (level)
        {
        case LogTrace:
            return "TRACE";
        case LogDebug:
            return "DEBUG";
        case LogInfo:
            return "INFO";
        case LogWarning:
            return "WARN";
        case LogError:
            return "ERROR";
        case LogOff:
            return "CLOSE";
        default:
            return "UNKNOWN";
        }
    }
    LogLevel Logger::getLevelFromString(const std::string& levelStr)
    {
        std::string levelUpper = levelStr;
        std::transform(levelUpper.begin(), levelUpper.end(), levelUpper.begin(),
            [](unsigned char c)
            { return std::toupper(c); });

        if (levelUpper == "TRACE")
            return LogTrace;
        if (levelUpper == "DEBUG")
            return LogDebug;
        if (levelUpper == "INFO")
            return LogInfo;
        if (levelUpper == "WARNING" || levelUpper == "WARN")
            return LogWarning;
        if (levelUpper == "ERROR")
            return LogError;
        if (levelUpper == "OFF")
            return LogOff;

        return LogInfo; // 默认级别
    }

} // namespace tdxpy