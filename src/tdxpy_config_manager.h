/**
 * @file        tdxpy_config_manager.h
 * @brief       通达信Python DLL配置管理器
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

#ifndef TDXPY_CONFIG_MANAGER_H
#define TDXPY_CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <json/json.h>
#include <tdxpy_logger.h>

 /// 默认配置文件名称
#define TDXPY_DEFAULT_CONFIG_FILE           "./T0002/dlls/config/tdxpy_config.json"

/// 默认Python主目录
#define TDXPY_DEFAULT_PYTHON_HOME           "./T0002/dlls/third_party/Python3142-32/"

/// 默认Python可执行文件
#define TDXPY_DEFAULT_PYTHON_EXECUTABLE     "./T0002/dlls/third_party/Python3142-32/python.exe"

/// 默认Python公式主目录
#define TDXPY_DEFAULT_PYTHON_FORMULAS_HOME  "./T0002/dlls/pythonenv/tdxpy_formulas/"

/// 默认Python虚拟环境主目录
#define TDXPY_DEFAULT_PYTHON_VENV_HOME      "./T0002/dlls/pythonenv/"

/// 默认log输出文件
#define TDXPY_DEFAULT_LOG_FILE              "./tdxpy_init.log"

namespace tdxpy {
    namespace config {

        /**
         * @brief 兼容性配置结构体（预留）
         */
        struct CompatibilityConfig {
            std::string tdxVersion;          ///< 通达信版本要求
            std::string pythonVersion;       ///< Python版本要求
            std::string architecture;        ///< 架构要求

            CompatibilityConfig() = default;
        };

        /**
         * @brief 元数据配置结构体（预留）
         */
        struct MetadataConfig {
            std::string configVersion;        ///< 配置文件版本
            CompatibilityConfig compatibility;///< 兼容性配置
            std::string created;              ///< 创建时间
            std::string updated;              ///< 更新时间
            std::string userParamsFormat;     ///< 用户参数格式说明

            MetadataConfig() = default;
        };

        /**
         * @brief 指标配置结构体
         */
        struct FormulaConfig {
            std::string name;                ///< 指标名称（如TDXPY_MA）
            std::string description;         ///< 指标描述
            std::string path;                ///< 路径（预留）
            std::string moduleName;          ///< Python模块名
            std::string function;            ///< Python函数名
            int id;                          ///< 指标ID(系统识别函数用，每个函数ID需唯一)
            std::string userParams;          ///< 用户参数（逗号分隔），预留参数

            FormulaConfig() : id(0) {}
        };

        /**
         * @brief 日志配置结构体
         */
        struct LoggingConfig {
            std::string logFile;             ///< 日志文件名
            std::string logLevel;            ///< 日志级别

            LoggingConfig() : logFile(TDXPY_DEFAULT_LOG_FILE), logLevel("DEBUG") {}
        };

        /**
         * @brief Python配置结构体
         */
        struct PythonConfig {
            std::string pythonHome;               ///< Python主目录
            std::string pythonExecutable;         ///< Python执行程序
            std::string pythonVenvHome;           ///< Python虚拟环境主目录
            std::string pythonFormulasHome;       ///< Python公式脚本目录
            std::vector<std::string> searchPaths; ///< 搜索路径
            bool enableDebug;                     ///< 是否启用调试

            PythonConfig() : enableDebug(false) {}
        };

        /**
         * @brief tdxpy_formulas基础配置结构体
         */
        struct TdxpyFormulasConfig {
            std::string version;             ///< 版本号
            std::string description;         ///< 描述
            std::string lastModified;        ///< 最后修改时间
            std::string author;              ///< 作者
            std::string license;             ///< 许可证

            TdxpyFormulasConfig() = default;
        };

        /**
         * @brief 配置管理器类
         */
        class ConfigManager {
        private:
            std::string m_configFilePath;    ///< 配置文件路径
            Json::Value m_root;              ///< JSON根节点
            bool m_isLoaded;                 ///< 是否已加载

            // 配置数据
            TdxpyFormulasConfig m_tdxpyFormulasConfig; ///< 基础配置
            PythonConfig m_pythonConfig;     ///< Python配置
            LoggingConfig m_loggingConfig;   ///< 日志配置
            MetadataConfig m_metadataConfig; ///< 元数据配置
            std::vector<FormulaConfig> m_formulaMappings; ///< 指标映射列表

            bool parseFormulaMapping(const Json::Value& formulaValue);
            bool validateJsonStructure() const;
            bool parseMetadataConfig(const Json::Value& metadataValue);

        public:
            /**
             * @brief 构造函数
             */
            ConfigManager() : m_isLoaded(false), m_configFilePath(TDXPY_DEFAULT_CONFIG_FILE) {}

            /**
             * @brief 构造函数（指定配置文件路径）
             * @param configPath 配置文件路径
             */
            explicit ConfigManager(const std::string& configPath);

            /**
             * @brief 加载配置文件
             * @param configPath 配置文件路径（为空则使用当前路径）
             * @return 加载成功返回true，失败返回false
             */
            bool load(const std::string& configPath = "");

            /**
             * @brief 重新加载配置文件
             * @return 加载成功返回true，失败返回false
             */
            bool reload();

            /**
             * @brief 获取版本号
             */
            const std::string& getVersion() const { return m_tdxpyFormulasConfig.version; }

            /**
             * @brief 获取描述信息
             */
            const std::string& getDescription() const { return m_tdxpyFormulasConfig.description; }

            /**
             * @brief 获取最后修改时间
             */
            const std::string& getLastModified() const { return m_tdxpyFormulasConfig.lastModified; }

            /**
             * @brief 获取作者
             */
            const std::string& getAuthor() const { return m_tdxpyFormulasConfig.author; }

            /**
             * @brief 获取许可证
             */
            const std::string& getLicense() const { return m_tdxpyFormulasConfig.license; }

            /**
             * @brief 获取Python配置
             */
            const PythonConfig& getPythonConfig() const { return m_pythonConfig; }

            /**
             * @brief 获取Python主目录
             */
            const std::string& getPythonHomePath() const { return m_pythonConfig.pythonHome; }

            /**
             * @brief 获取Python执行程序
             */
            const std::string& getPythonExecutableFile() const { return m_pythonConfig.pythonExecutable; }

             /**
             * @brief 获取Python虚拟环境根目录
             */
            const std::string& getPythonVenvHome() const { return m_pythonConfig.pythonVenvHome; }


            /**
             * @brief 获取Python库搜索路径数组
             */
            const std::vector<std::string>& getPythonSearchPaths() const { return m_pythonConfig.searchPaths; }

            /**
             * @brief 获取Pythong公式脚本目录
             */
            const  std::string& getpythonFormulasHomePath() const { return m_pythonConfig.pythonFormulasHome; }

            /**
             * @brief 获取日志配置
             */
            const LoggingConfig& getLoggingConfig() const { return m_loggingConfig; }

            /**
             * @brief 获取日志文件名
             */
            const std::string& getLoggingLogFile() const { return m_loggingConfig.logFile; }

            /**
            * @brief 获取日志级别
            */
            const std::string& getLoggingLogLevel() const { return m_loggingConfig.logLevel; }

            /**
             * @brief 获取元数据配置
             */
            const MetadataConfig& getMetadataConfig() const { return m_metadataConfig; }

            /**
             * @brief 获取基础配置
             */
            const TdxpyFormulasConfig& getTdxpyFormulasConfig() const { return m_tdxpyFormulasConfig; }

            /**
             * @brief 获取所有指标配置
             */
            const std::vector<FormulaConfig>& getFormulaMappings() const { return m_formulaMappings; }

            /**
             * @brief 根据ID获取指标配置
             * @param id 指标ID
             * @return 指标配置指针，不存在返回nullptr
             */
            const FormulaConfig* getFormulaById(int id) const;

            /**
             * @brief 根据名称获取指标配置
             * @param name 指标名称（如TDXPY_MA）
             * @return 指标配置指针，不存在返回nullptr
             */
            const FormulaConfig* getFormulaByName(const std::string& name) const;

            /**
             * @brief 检查是否包含指定指标
             * @param name 指标名称
             */
            bool containsFormula(const std::string& name) const {
                return getFormulaByName(name) != nullptr;
            }

            /**
             * @brief 获取所有指标名称列表
             */
            std::vector<std::string> getFormulaNames() const;

            /**
             * @brief 验证配置完整性
             * @return 配置完整返回true，否则返回false
             */
            bool validate() const;

            /**
             * @brief 检查配置是否已加载
             */
            bool isLoaded() const { return m_isLoaded; }

            /**
             * @brief 获取配置文件路径
             */
            const std::string& getConfigFilePath() const { return m_configFilePath; }

            /**
             * @brief 打印配置摘要
             */
            void printSummary() const;

            /**
             * @brief 打印详细配置信息
             */
            void printDetailedInfo() const;

            /**
             * @brief 生成默认配置文件内容（适配新版格式）
             * @return 默认配置JSON字符串
             */
            static std::string generateDefaultConfig();

            /**
             * @brief 保存配置到文件
             * @param filePath 文件路径
             * @return 保存成功返回true，失败返回false
             */
            bool saveToFile(const std::string& filePath) const;
        };

    } // namespace config
} // namespace tdxpy

#endif // TDXPY_CONFIG_MANAGER_H#pragma once
