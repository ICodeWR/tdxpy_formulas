/**
 * @file        tdxpy_config_manager.cpp
 * @brief       通达信Python DLL配置管理器实现
 * @author      码上工坊
 * @copyright   Copyright (c) 2026-2030 码上工坊 Contributors
 * @license     MIT License (详见项目根目录LICENSE文件)
 * @version     0.1.0
 * @date        2026-01-08
 *
 * @par 修改记录:
 * <table>
 * <tr><th>日期         <th>版本      <th>作者              <th>描述
 * <tr><td>2026-01-08   <td>0.1.0    <td>码上工坊           <td>初始版本
 * </table>
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "tdxpy_logger.h"
#include "tdxpy_env_var.h"
#include "tdxpy_config_manager.h"

namespace tdxpy
{
    namespace config
    {
        // 构造函数
        ConfigManager::ConfigManager(const std::string &configPath)
            : m_isLoaded(false),
              m_configFilePath(TDXPY_DEFAULT_CONFIG_FILE)
        {
            m_isLoaded = false;

            // 1. 优先使用传入的参数
            if (!configPath.empty())
            {
                m_configFilePath = configPath;
                TDXPY_LOG_DEBUG(u8"使用指定配置文件: " + configPath);
            }
            else
            {
                // 2. 尝试从环境变量获取
                std::string envPath = tdxpy::utils::EnvVarManager::get("TDXPY_CONFIG_FILE").value_or("TDXPY_DEFAULT_CONFIG_FILE");
                if (!envPath.empty())
                {
                    m_configFilePath = envPath;
                    TDXPY_LOG_DEBUG(u8"使用环境变量配置文件: " + envPath);
                }
                else
                {
                    // 3. 使用默认配置
                    m_configFilePath = TDXPY_DEFAULT_CONFIG_FILE;
                    TDXPY_LOG_DEBUG(u8"使用默认配置文件: " + m_configFilePath);
                }
            }

            // 确保路径不为空
            if (m_configFilePath.empty())
            {
                TDXPY_LOG_WARNING(u8"警告: 配置文件路径为空，使用默认路径");
                m_configFilePath = TDXPY_DEFAULT_CONFIG_FILE;
            }
        }

        bool ConfigManager::load(const std::string &configPath)
        {
            if (!configPath.empty())
            {
                m_configFilePath = configPath;
                TDXPY_LOG_DEBUG(u8"加载指定配置文件: " + configPath);
            }

            TDXPY_LOG_DEBUG(u8"配置文件路径: " + m_configFilePath);

            // 检查文件是否存在
            if (!std::filesystem::exists(m_configFilePath))
            {
                TDXPY_LOG_ERROR(u8"错误: 配置文件不存在 - " + m_configFilePath);
                return false;
            }

            std::ifstream configFileStream(m_configFilePath);
            if (!configFileStream.is_open())
            {
                TDXPY_LOG_ERROR(u8"错误: 无法打开配置文件 - " + m_configFilePath);
                return false;
            }

            Json::CharReaderBuilder jsonReaderBuilder;
            std::string errs;

            TDXPY_LOG_DEBUG(u8"开始解析配置文件: " + m_configFilePath);

            // 解析JSON
            if (!Json::parseFromStream(jsonReaderBuilder, configFileStream, &m_root, &errs))
            {
                TDXPY_LOG_ERROR(u8"错误: JSON解析失败 - " + errs);
                return false;
            }

            configFileStream.close();

            // 验证JSON结构
            if (!validateJsonStructure())
            {
                TDXPY_LOG_ERROR(u8"错误: 配置文件结构无效");
                return false;
            }

            try
            {
                // 解析基础配置
                Json::Value tdxpyFormulas = m_root["tdxpy_formulas"];
                m_tdxpyFormulasConfig.version = tdxpyFormulas["version"].asString();
                m_tdxpyFormulasConfig.description = tdxpyFormulas["description"].asString();
                m_tdxpyFormulasConfig.lastModified = tdxpyFormulas["last_modified"].asString();
                m_tdxpyFormulasConfig.author = tdxpyFormulas["author"].asString();
                m_tdxpyFormulasConfig.license = tdxpyFormulas["license"].asString();

                TDXPY_LOG_DEBUG(u8"版本: " + m_tdxpyFormulasConfig.version);
                TDXPY_LOG_DEBUG(u8"描述: " + m_tdxpyFormulasConfig.description);
                TDXPY_LOG_DEBUG(u8"最后修改: " + m_tdxpyFormulasConfig.lastModified);
                TDXPY_LOG_DEBUG(u8"作者: " + m_tdxpyFormulasConfig.author);
                TDXPY_LOG_DEBUG(u8"许可证: " + m_tdxpyFormulasConfig.license);

                // 解析Python配置
                Json::Value pyConfig = m_root["python_config"];
                m_pythonConfig.pythonHome = pyConfig["python_home"].asString();
                m_pythonConfig.pythonVenvHome = pyConfig["python_venv_home"].asString();
                m_pythonConfig.pythonExecutable = pyConfig["python_executable"].asString();
                m_pythonConfig.pythonFormulasHome = pyConfig["python_formulas_home"].asString();
                m_pythonConfig.enableDebug = pyConfig["enable_debug"].asBool();

                // 解析搜索路径数组
                const Json::Value &searchPaths = pyConfig["search_paths"];
                if (searchPaths.isArray())
                {
                    m_pythonConfig.searchPaths.clear();
                    for (const auto &path : searchPaths)
                    {
                        m_pythonConfig.searchPaths.push_back(path.asString());
                    }
                }

                TDXPY_LOG_DEBUG(u8"Python主目录: " + m_pythonConfig.pythonHome);
                TDXPY_LOG_DEBUG(u8"Python可执行文件: " + m_pythonConfig.pythonExecutable);
                TDXPY_LOG_DEBUG(u8"Python公式目录: " + m_pythonConfig.pythonFormulasHome);
                TDXPY_LOG_DEBUG(u8"启用调试: " + std::string(m_pythonConfig.enableDebug ? u8"是" : u8"否"));
                TDXPY_LOG_DEBUG(u8"搜索路径数量: " + std::to_string(m_pythonConfig.searchPaths.size()));

                TDXPY_LOG_DEBUG(u8"===========================================");
                for (const auto &path : m_pythonConfig.searchPaths)
                {
                    TDXPY_LOG_DEBUG(u8"搜索目录: " + path);
                }
                TDXPY_LOG_DEBUG(u8"===========================================");

                // 解析日志配置
                Json::Value logConfig = m_root["logging_config"];
                m_loggingConfig.logFile = logConfig["log_file"].asString();
                m_loggingConfig.logLevel = logConfig["log_level"].asString();

                TDXPY_LOG_DEBUG(u8"日志文件: " + m_loggingConfig.logFile);
                TDXPY_LOG_DEBUG(u8"日志级别: " + m_loggingConfig.logLevel);

                // 解析元数据配置
                if (m_root.isMember("metadata"))
                {
                    parseMetadataConfig(m_root["metadata"]);
                }

                // 解析公式映射数组
                m_formulaMappings.clear();
                Json::Value mappings = m_root["formula_mappings"];
                if (mappings.isArray())
                {
                    for (const auto &mapping : mappings)
                    {
                        if (!parseFormulaMapping(mapping))
                        {
                            TDXPY_LOG_ERROR(u8"警告: 解析公式映射失败");
                        }
                    }
                }

                TDXPY_LOG_DEBUG(u8"公式映射数量: " + std::to_string(m_formulaMappings.size()));

                m_isLoaded = true;
                return true;
            }
            catch (const std::exception &e)
            {
                TDXPY_LOG_ERROR(u8"错误: 解析配置数据异常 - " + std::string(e.what()));
                m_isLoaded = false;
                return false;
            }
        }

        bool ConfigManager::reload()
        {
            TDXPY_LOG_DEBUG(u8"重新加载配置文件: " + m_configFilePath);
            return load(m_configFilePath);
        }

        bool ConfigManager::parseMetadataConfig(const Json::Value &metadataValue)
        {
            if (!metadataValue.isObject())
            {
                TDXPY_LOG_ERROR(u8"元数据配置不是有效的对象");
                return false;
            }

            m_metadataConfig.configVersion = metadataValue.get("config_version", "").asString();
            m_metadataConfig.created = metadataValue.get("created", "").asString();
            m_metadataConfig.updated = metadataValue.get("updated", "").asString();
            m_metadataConfig.userParamsFormat = metadataValue.get("user_params_format", "").asString();

            // 解析兼容性配置
            Json::Value compatibility = metadataValue["compatibility"];
            if (compatibility.isObject())
            {
                m_metadataConfig.compatibility.tdxVersion = compatibility.get("tdx_version", "").asString();
                m_metadataConfig.compatibility.pythonVersion = compatibility.get("python_version", "").asString();
                m_metadataConfig.compatibility.architecture = compatibility.get("architecture", "").asString();
            }

            TDXPY_LOG_DEBUG(u8"配置版本: " + m_metadataConfig.configVersion);
            TDXPY_LOG_DEBUG(u8"通达信兼容版本: " + m_metadataConfig.compatibility.tdxVersion);
            TDXPY_LOG_DEBUG(u8"Python兼容版本: " + m_metadataConfig.compatibility.pythonVersion);

            return true;
        }

        bool ConfigManager::parseFormulaMapping(const Json::Value &formulaValue)
        {
            try
            {
                FormulaConfig config;

                config.name = formulaValue.get("name", "").asString();
                config.description = formulaValue.get("description", "").asString();
                config.path = formulaValue.get("path", "").asString();
                config.moduleName = formulaValue.get("module_name", "").asString();
                config.function = formulaValue.get("function", "").asString();
                config.id = formulaValue.get("id", 0).asInt();
                config.userParams = formulaValue.get("user_params", "").asString();

                // 验证必要字段
                if (config.name.empty() || config.function.empty() || config.id <= 0)
                {
                    TDXPY_LOG_ERROR(u8"警告: 公式映射配置不完整: " + config.name);
                    return false;
                }

                // 检查名称前缀是否符合规范
                if (config.name.find("TDXPY_") != 0)
                {
                    TDXPY_LOG_DEBUG(u8"警告: 公式名称 '" + config.name + u8"' 应该以 'TDXPY_' 开头");
                }

                TDXPY_LOG_DEBUG(u8"---------------------------------");
                TDXPY_LOG_DEBUG(u8"公式名称：" + config.name);
                TDXPY_LOG_DEBUG(u8"公式描述：" + config.description);
                TDXPY_LOG_DEBUG(u8"公式编号：" + std::to_string(config.id));
                TDXPY_LOG_DEBUG(u8"公式路径：" + config.path);
                TDXPY_LOG_DEBUG(u8"模块名称：" + config.moduleName);
                TDXPY_LOG_DEBUG(u8"函数名称：" + config.function);
                TDXPY_LOG_DEBUG(u8"用户参数：" + config.userParams);
                TDXPY_LOG_DEBUG(u8"---------------------------------");

                m_formulaMappings.push_back(config);
                return true;
            }
            catch (const std::exception &e)
            {
                TDXPY_LOG_ERROR(u8"解析公式映射异常: " + std::string(e.what()));
                return false;
            }
        }

        const FormulaConfig *ConfigManager::getFormulaById(int id) const
        {
            for (const auto &config : m_formulaMappings)
            {
                if (config.id == id)
                {
                    return &config;
                }
            }
            return nullptr;
        }

        const FormulaConfig *ConfigManager::getFormulaByName(const std::string &name) const
        {
            for (const auto &config : m_formulaMappings)
            {
                if (config.name == name)
                {
                    return &config;
                }
            }
            return nullptr;
        }

        std::vector<std::string> ConfigManager::getFormulaNames() const
        {
            std::vector<std::string> names;
            names.reserve(m_formulaMappings.size());

            for (const auto &config : m_formulaMappings)
            {
                names.push_back(config.name);
            }

            return names;
        }

        bool ConfigManager::validate() const
        {
            if (!m_isLoaded)
            {
                TDXPY_LOG_ERROR(u8"配置未加载");
                return false;
            }

            // 检查基础配置
            if (m_tdxpyFormulasConfig.version.empty() ||
                m_tdxpyFormulasConfig.description.empty())
            {
                TDXPY_LOG_ERROR(u8"基础配置不完整");
                return false;
            }

            // 检查Python配置
            if (m_pythonConfig.pythonHome.empty() ||
                m_pythonConfig.pythonExecutable.empty())
            {
                TDXPY_LOG_ERROR(u8"Python配置不完整");
                return false;
            }

            // 检查公式映射
            if (m_formulaMappings.empty())
            {
                TDXPY_LOG_WARNING(u8"警告: 没有找到公式映射配置");
            }

            return true;
        }

        void ConfigManager::printSummary() const
        {
            if (!m_isLoaded)
            {
                std::cout << u8"配置未加载" << std::endl;
                return;
            }

            std::cout << u8"=== TDXPY配置摘要 ===" << std::endl;
            std::cout << u8"版本: " << m_tdxpyFormulasConfig.version << std::endl;
            std::cout << u8"描述: " << m_tdxpyFormulasConfig.description << std::endl;
            std::cout << u8"作者: " << m_tdxpyFormulasConfig.author << std::endl;
            std::cout << u8"Python版本: " << m_metadataConfig.compatibility.pythonVersion << std::endl;
            std::cout << u8"公式数量: " << m_formulaMappings.size() << std::endl;
            std::cout << u8"===================" << std::endl;
        }

        void ConfigManager::printDetailedInfo() const
        {
            if (!m_isLoaded)
            {
                std::cout << u8"配置未加载" << std::endl;
                return;
            }

            std::cout << std::left;

            // 基础配置
            std::cout << u8"\n--- 基础配置 ---" << std::endl;
            std::cout << std::setw(15) << u8"版本" << ": " << m_tdxpyFormulasConfig.version << std::endl;
            std::cout << std::setw(15) << u8"描述" << ": " << m_tdxpyFormulasConfig.description << std::endl;
            std::cout << std::setw(15) << u8"最后修改" << ": " << m_tdxpyFormulasConfig.lastModified << std::endl;
            std::cout << std::setw(15) << u8"作者" << ": " << m_tdxpyFormulasConfig.author << std::endl;
            std::cout << std::setw(15) << u8"许可证" << ": " << m_tdxpyFormulasConfig.license << std::endl;

            // 元数据配置
            std::cout << u8"\n--- 元数据配置 ---" << std::endl;
            std::cout << std::setw(15) << u8"配置版本" << ": " << m_metadataConfig.configVersion << std::endl;
            std::cout << std::setw(15) << u8"创建时间" << ": " << m_metadataConfig.created << std::endl;
            std::cout << std::setw(15) << u8"更新时间" << ": " << m_metadataConfig.updated << std::endl;
            std::cout << std::setw(15) << u8"参数格式" << ": " << m_metadataConfig.userParamsFormat << std::endl;
            std::cout << std::setw(15) << u8"通达信版本" << ": " << m_metadataConfig.compatibility.tdxVersion << std::endl;
            std::cout << std::setw(15) << u8"Python版本" << ": " << m_metadataConfig.compatibility.pythonVersion << std::endl;
            std::cout << std::setw(15) << u8"架构" << ": " << m_metadataConfig.compatibility.architecture << std::endl;

            // Python配置
            std::cout << u8"\n--- Python配置 ---" << std::endl;
            std::cout << std::setw(20) << u8"Python主目录" << ": " << m_pythonConfig.pythonHome << std::endl;
            std::cout << std::setw(20) << u8"Python可执行文件" << ": " << m_pythonConfig.pythonExecutable << std::endl;
            std::cout << std::setw(20) << u8"公式主目录" << ": " << m_pythonConfig.pythonFormulasHome << std::endl;
            std::cout << std::setw(20) << u8"调试模式" << ": " << (m_pythonConfig.enableDebug ? u8"启用" : u8"禁用") << std::endl;

            std::cout << std::setw(20) << u8"搜索路径" << ": " << std::endl;
            for (size_t i = 0; i < m_pythonConfig.searchPaths.size(); ++i)
            {
                std::cout << "    [" << i + 1 << "] " << m_pythonConfig.searchPaths[i] << std::endl;
            }

            // 日志配置
            std::cout << u8"\n--- 日志配置 ---" << std::endl;
            std::cout << std::setw(20) << u8"日志文件" << ": " << m_loggingConfig.logFile << std::endl;
            std::cout << std::setw(20) << u8"日志级别" << ": " << m_loggingConfig.logLevel << std::endl;

            // 公式映射
            std::cout << u8"\n--- 公式映射 (" << m_formulaMappings.size() << "个) ---" << std::endl;
            for (const auto &config : m_formulaMappings)
            {
                std::cout << "\n[" << config.name << "] (ID: " << config.id << ")" << std::endl;
                std::cout << std::setw(15) << u8"  描述" << ": " << config.description << std::endl;
                std::cout << std::setw(15) << u8"  模块" << ": " << config.moduleName << std::endl;
                std::cout << std::setw(15) << u8"  函数" << ": " << config.function << std::endl;
                std::cout << std::setw(15) << u8"  用户参数" << ": " << config.userParams << std::endl;
            }

            // 状态信息
            std::cout << u8"\n--- 状态信息 ---" << std::endl;
            std::cout << std::setw(15) << u8"配置文件" << ": " << m_configFilePath << std::endl;
            std::cout << std::setw(15) << u8"加载状态" << ": " << (m_isLoaded ? u8"已加载" : u8"未加载") << std::endl;
            std::cout << std::setw(15) << u8"验证状态" << ": " << (validate() ? u8"有效" : u8"无效") << std::endl;
        }

        std::string ConfigManager::generateDefaultConfig()
        {
            Json::Value root;

            // tdxpy_formulas
            Json::Value tdxpyFormulas;
            tdxpyFormulas["version"] = "0.1.0";
            tdxpyFormulas["description"] = u8"通达信Python公式插件配置文件";
            tdxpyFormulas["last_modified"] = "2026-01-05";
            tdxpyFormulas["author"] = u8"码上工坊";
            tdxpyFormulas["license"] = "MIT";
            root["tdxpy_formulas"] = tdxpyFormulas;

            // python_config
            Json::Value pythonConfig;
            pythonConfig["python_home"] = TDXPY_DEFAULT_PYTHON_HOME;
            pythonConfig["python_executable"] = TDXPY_DEFAULT_PYTHON_EXECUTABLE;
            pythonConfig["python_formulas_home"] = TDXPY_DEFAULT_PYTHON_FORMULAS_HOME;

            Json::Value searchPaths(Json::arrayValue);
            searchPaths.append(TDXPY_DEFAULT_PYTHON_HOME);
            searchPaths.append(std::string(TDXPY_DEFAULT_PYTHON_HOME) + "Lib");
            searchPaths.append(std::string(TDXPY_DEFAULT_PYTHON_HOME) + "Dlls");
            searchPaths.append(std::string(TDXPY_DEFAULT_PYTHON_VENV_HOME) + "Lib");
            searchPaths.append(std::string(TDXPY_DEFAULT_PYTHON_VENV_HOME) + "Lib/site-packages");
            searchPaths.append(TDXPY_DEFAULT_PYTHON_FORMULAS_HOME);
            searchPaths.append("./T0002/dlls");
            searchPaths.append("./");
            pythonConfig["search_paths"] = searchPaths;

            pythonConfig["enable_debug"] = false;
            root["python_config"] = pythonConfig;

            // logging_config
            Json::Value loggingConfig;
            loggingConfig["log_file"] = TDXPY_DEFAULT_LOG_FILE;
            loggingConfig["log_level"] = "LogWarning";
            root["logging_config"] = loggingConfig;

            // formula_mappings (数组格式)
            Json::Value formulaMappings(Json::arrayValue);

            Json::Value maMapping;
            maMapping["name"] = "TDXPY_MA";
            maMapping["description"] = u8"移动平均线";
            maMapping["path"] = "";
            maMapping["module_name"] = "ma_indicator";
            maMapping["function"] = "calculate_ma";
            maMapping["id"] = 1;
            maMapping["user_params"] = "5,10,20,60";
            maMapping["input_count"] = 1;
            formulaMappings.append(maMapping);

            root["formula_mappings"] = formulaMappings;

            // metadata
            Json::Value metadata;
            metadata["config_version"] = "0.1.0";

            Json::Value compatibility;
            compatibility["tdx_version"] = ">=7.0";
            compatibility["python_version"] = "3.14.2";
            compatibility["architecture"] = "32bit";
            metadata["compatibility"] = compatibility;

            metadata["created"] = "2026-01-05";
            metadata["updated"] = "2026-01-08";
            metadata["user_params_format"] = u8"逗号分隔的字符串，如'5,10,20'表示多个周期参数";
            root["metadata"] = metadata;

            Json::StreamWriterBuilder writer;
            writer["indentation"] = "    ";
            writer["commentStyle"] = "None";

            return Json::writeString(writer, root);
        }

        bool ConfigManager::saveToFile(const std::string &filePath) const
        {
            if (!m_isLoaded)
            {
                TDXPY_LOG_ERROR(u8"配置未加载，无法保存");
                return false;
            }

            std::string savePath = filePath.empty() ? m_configFilePath : filePath;

            std::ofstream outFile(savePath);
            if (!outFile.is_open())
            {
                TDXPY_LOG_ERROR(u8"无法创建文件: " + savePath);
                return false;
            }

            Json::StreamWriterBuilder writer;
            writer["indentation"] = "    ";
            writer["commentStyle"] = "None";

            std::unique_ptr<Json::StreamWriter> jsonWriter(writer.newStreamWriter());
            jsonWriter->write(m_root, &outFile);

            outFile.close();
            TDXPY_LOG_DEBUG(u8"配置保存成功: " + savePath);
            return true;
        }

        bool ConfigManager::validateJsonStructure() const
        {
            // 检查必需字段
            if (!m_root.isMember("tdxpy_formulas") ||
                !m_root.isMember("python_config") ||
                !m_root.isMember("logging_config") ||
                !m_root.isMember("formula_mappings"))
            {
                TDXPY_LOG_ERROR(u8"缺少必需字段");
                return false;
            }

            // 检查tdxpy_formulas结构
            Json::Value formulas = m_root["tdxpy_formulas"];
            if (!formulas.isMember("version") ||
                !formulas.isMember("description") ||
                !formulas.isMember("author"))
            {
                TDXPY_LOG_ERROR(u8"tdxpy_formulas结构不完整");
                return false;
            }

            // 检查formula_mappings是数组
            Json::Value mappings = m_root["formula_mappings"];
            if (!mappings.isArray())
            {
                TDXPY_LOG_ERROR(u8"formula_mappings应该是数组");
                return false;
            }

            return true;
        }

    } // namespace config
} // namespace tdxpy