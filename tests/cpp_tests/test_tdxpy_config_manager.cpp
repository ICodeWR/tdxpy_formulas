/**
 * @file        test_tdxpy_config_manager.cpp
 * @brief       tdxpy_config_manager 单元测试
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

#include <filesystem>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include "gtest/gtest.h"
#include "tdxpy_config_manager.h"
#include "tdxpy_env_var.h"

namespace fs = std::filesystem;

class TdxpyConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录
        testDir = "test_config_temp";
        fs::create_directories(testDir);
        
        // 设置测试配置文件路径
        testConfigFile = testDir + "/tdxpy_config_test.json";
        invalidConfigFile = testDir + "/invalid_config.json";
        emptyConfigFile = testDir + "/empty_config.json";
        
        // 清除可能存在的环境变量
        tdxpy::utils::EnvVarManager::remove("TDXPY_CONFIG_FILE");
        
        // 创建测试配置文件
        createValidTestConfig();
        createInvalidTestConfig();
        createEmptyTestConfig();
    }

    void TearDown() override {
        // 清理环境变量
        tdxpy::utils::EnvVarManager::remove("TDXPY_CONFIG_FILE");
        
        // 删除测试目录
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    std::string testDir;
    std::string testConfigFile;
    std::string invalidConfigFile;
    std::string emptyConfigFile;
    
    // 创建有效的测试配置文件
    void createValidTestConfig() {
        std::ofstream file(testConfigFile);
        if (!file.is_open()) {
            FAIL() << "无法创建测试配置文件";
        }
        
        file << R"({
    "tdxpy_formulas": {
        "version": "1.2.3",
        "description": "测试配置文件",
        "last_modified": "2026-01-12",
        "author": "测试作者",
        "license": "MIT"
    },
    "python_config": {
        "python_home": "./third_party/Python3142-32/",
        "python_venv_home": "./pythonenv/",
        "python_executable": "./third_party/Python3142-32/python.exe",
        "python_formulas_home": "./pythonenv/tdxpy_formulas/",
        "enable_debug": true,
        "search_paths": [
            "./third_party/Python3142-32/",
            "./third_party/Python3142-32/Lib",
            "./third_party/Python3142-32/Dlls",
            "./pythonenv/Lib",
            "./pythonenv/Lib/site-packages",
            "./pythonenv/tdxpy_formulas/",
            "./T0002/dlls",
            "./"
        ]
    },
    "logging_config": {
        "log_file": "./test_tdxpy.log",
        "log_level": "DEBUG"
    },
    "metadata": {
        "config_version": "1.0.0",
        "compatibility": {
            "tdx_version": ">=7.0",
            "python_version": "3.14.2",
            "architecture": "32bit"
        },
        "created": "2026-01-12",
        "updated": "2026-01-12",
        "user_params_format": "逗号分隔参数，如'5,10,20'"
    },
    "formula_mappings": [
        {
            "name": "TDXPY_MA",
            "description": "移动平均线测试",
            "path": "./pythonenv/tdxpy_formulas/",
            "module_name": "ma_indicator",
            "function": "calculate_ma",
            "id": 1,
            "user_params": "5,10,20,60"
        },
        {
            "name": "TDXPY_RSI",
            "description": "相对强弱指数",
            "path": "./pythonenv/tdxpy_formulas/",
            "module_name": "rsi_indicator",
            "function": "calculate_rsi",
            "id": 2,
            "user_params": "14"
        },
        {
            "name": "TDXPY_MACD",
            "description": "指数平滑异同平均线",
            "path": "./pythonenv/tdxpy_formulas/",
            "module_name": "macd_indicator",
            "function": "calculate_macd",
            "id": 3,
            "user_params": "12,26,9"
        },
        {
            "name": "TDXPY_BOLL",
            "description": "布林带指标",
            "path": "./pythonenv/tdxpy_formulas/",
            "module_name": "boll_indicator",
            "function": "calculate_boll",
            "id": 4,
            "user_params": "20"
        }
    ]
})";
        file.close();
    }
    
    // 创建无效的测试配置文件
    void createInvalidTestConfig() {
        std::ofstream file(invalidConfigFile);
        if (!file.is_open()) {
            FAIL() << "无法创建无效测试配置文件";
        }
        
        file << R"({
    "tdxpy_formulas": {
        // 缺少必需字段
    },
    "python_config": {
        // 无效的JSON结构
    },
    "formula_mappings": "不是数组"
})";
        file.close();
    }
    
    // 创建空的配置文件
    void createEmptyTestConfig() {
        std::ofstream file(emptyConfigFile);
        if (!file.is_open()) {
            FAIL() << "无法创建空测试配置文件";
        }
        file.close();
    }
    
    // 生成随机配置文件名
    std::string generateRandomConfigFileName() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        return testDir + "/config_random_" + std::to_string(dis(gen)) + ".json";
    }
    
    // 验证配置管理器状态
    void validateConfigState(const tdxpy::config::ConfigManager& config, 
                           bool shouldBeLoaded, 
                           const std::string& expectedPath = "") {
        EXPECT_EQ(config.isLoaded(), shouldBeLoaded);
        
        if (!expectedPath.empty()) {
            EXPECT_EQ(config.getConfigFilePath(), expectedPath);
        }
    }
};

// 测试1: 默认构造函数
TEST_F(TdxpyConfigManagerTest, Constructor_Default) {
    tdxpy::config::ConfigManager config;
    
    EXPECT_FALSE(config.isLoaded());
    EXPECT_TRUE(config.getConfigFilePath().empty());
    
    // 验证默认值
    EXPECT_TRUE(config.getPythonHomePath().empty());
    EXPECT_TRUE(config.getPythonExecutableFile().empty());
    EXPECT_FALSE(config.getLoggingLogFile().empty());
    EXPECT_EQ(config.getLoggingLogFile(), TDXPY_DEFAULT_LOG_FILE);
}

// 测试2: 带路径参数的构造函数
TEST_F(TdxpyConfigManagerTest, Constructor_WithPath) {
    tdxpy::config::ConfigManager config(testConfigFile);
    
    EXPECT_FALSE(config.isLoaded());
    EXPECT_EQ(config.getConfigFilePath(), testConfigFile);
}

// 测试3: 加载有效的配置文件
TEST_F(TdxpyConfigManagerTest, Load_ValidConfigFile) {
    tdxpy::config::ConfigManager config(testConfigFile);
    
    bool result = config.load();
    EXPECT_TRUE(result);
    EXPECT_TRUE(config.isLoaded());
    
    // 验证基础配置
    EXPECT_EQ(config.getVersion(), "1.2.3");
    EXPECT_EQ(config.getDescription(), "测试配置文件");
    EXPECT_EQ(config.getAuthor(), "测试作者");
    EXPECT_EQ(config.getLicense(), "MIT");
    EXPECT_EQ(config.getLastModified(), "2026-01-12");
    
    // 验证Python配置
    EXPECT_EQ(config.getPythonHomePath(), "./third_party/Python3142-32/");
    EXPECT_EQ(config.getPythonExecutableFile(), "./third_party/Python3142-32/python.exe");
    EXPECT_EQ(config.getPythonVenvHome(), "./pythonenv/");
    EXPECT_EQ(config.getpythonFormulasHomePath(), "./pythonenv/tdxpy_formulas/");
    EXPECT_TRUE(config.getPythonConfig().enableDebug);
    
    // 验证搜索路径
    const auto& searchPaths = config.getPythonSearchPaths();
    EXPECT_EQ(searchPaths.size(), 8);
    EXPECT_EQ(searchPaths[0], "./third_party/Python3142-32/");
    EXPECT_EQ(searchPaths[7], "./");
    
    // 验证日志配置
    EXPECT_EQ(config.getLoggingLogFile(), "./test_tdxpy.log");
    EXPECT_EQ(config.getLoggingLogLevel(), "DEBUG");
    
    // 验证元数据配置
    const auto& metadata = config.getMetadataConfig();
    EXPECT_EQ(metadata.configVersion, "1.0.0");
    EXPECT_EQ(metadata.compatibility.tdxVersion, ">=7.0");
    EXPECT_EQ(metadata.compatibility.pythonVersion, "3.14.2");
    EXPECT_EQ(metadata.compatibility.architecture, "32bit");
    EXPECT_EQ(metadata.userParamsFormat, "逗号分隔参数，如'5,10,20'");
    
    // 验证公式映射
    const auto& formulas = config.getFormulaMappings();
    EXPECT_EQ(formulas.size(), 4);
    
    // 验证第一个公式
    EXPECT_EQ(formulas[0].name, "TDXPY_MA");
    EXPECT_EQ(formulas[0].description, "移动平均线测试");
    EXPECT_EQ(formulas[0].moduleName, "ma_indicator");
    EXPECT_EQ(formulas[0].function, "calculate_ma");
    EXPECT_EQ(formulas[0].id, 1);
    EXPECT_EQ(formulas[0].userParams, "5,10,20,60");
    
    // 验证最后一个公式
    EXPECT_EQ(formulas[3].name, "TDXPY_BOLL");
    EXPECT_EQ(formulas[3].id, 4);
}

// 测试4: 加载不存在的配置文件
TEST_F(TdxpyConfigManagerTest, Load_NonExistentFile) {
    std::string nonExistentFile = testDir + "/non_existent_config.json";
    tdxpy::config::ConfigManager config(nonExistentFile);
    
    bool result = config.load();
    EXPECT_FALSE(result);
    EXPECT_FALSE(config.isLoaded());
}

// 测试5: 加载无效的配置文件
TEST_F(TdxpyConfigManagerTest, Load_InvalidConfigFile) {
    tdxpy::config::ConfigManager config(invalidConfigFile);
    
    bool result = config.load();
    EXPECT_FALSE(result);
    EXPECT_FALSE(config.isLoaded());
}

// 测试6: 加载空配置文件
TEST_F(TdxpyConfigManagerTest, Load_EmptyConfigFile) {
    tdxpy::config::ConfigManager config(emptyConfigFile);
    
    bool result = config.load();
    EXPECT_FALSE(result);
    EXPECT_FALSE(config.isLoaded());
}

// 测试7: 使用参数加载配置文件
TEST_F(TdxpyConfigManagerTest, Load_WithParameter) {
    tdxpy::config::ConfigManager config; // 使用默认路径
    
    // 使用参数加载不同的配置文件
    bool result = config.load(testConfigFile);
    EXPECT_TRUE(result);
    EXPECT_TRUE(config.isLoaded());
    EXPECT_EQ(config.getConfigFilePath(), testConfigFile);
}

// 测试8: 重新加载配置文件
TEST_F(TdxpyConfigManagerTest, Reload_ConfigFile) {
    tdxpy::config::ConfigManager config(testConfigFile);
    
    // 第一次加载
    bool result1 = config.load();
    EXPECT_TRUE(result1);
    
    // 修改配置文件
    std::ofstream file(testConfigFile, std::ios::app);
    file << "\n// 修改后的配置文件";
    file.close();
    
    // 重新加载
    bool result2 = config.reload();
    EXPECT_TRUE(result2);
    EXPECT_TRUE(config.isLoaded());
}

// 测试9: 从环境变量加载配置
TEST_F(TdxpyConfigManagerTest, Load_FromEnvironmentVariable) {
    // 设置环境变量
    bool envSet = tdxpy::utils::EnvVarManager::set("TDXPY_CONFIG_FILE", testConfigFile);
    ASSERT_TRUE(envSet);
    
    // 使用默认构造函数，应该从环境变量读取路径
    tdxpy::config::ConfigManager config;
    
    bool result = config.load();
    EXPECT_TRUE(result);
    EXPECT_TRUE(config.isLoaded());
    EXPECT_EQ(config.getConfigFilePath(), testConfigFile);
}

// 测试10: 参数优先于环境变量
TEST_F(TdxpyConfigManagerTest, Load_ParameterPriorityOverEnvVar) {
    // 设置环境变量指向一个文件
    std::string envConfigFile = generateRandomConfigFileName();
    std::ofstream envFile(envConfigFile);
    envFile << R"({"tdxpy_formulas": {"version": "env_version"}})";
    envFile.close();
    
    bool envSet = tdxpy::utils::EnvVarManager::set("TDXPY_CONFIG_FILE", envConfigFile);
    ASSERT_TRUE(envSet);
    
    // 使用参数加载不同的配置文件
    tdxpy::config::ConfigManager config(testConfigFile);
    bool result = config.load();
    
    EXPECT_TRUE(result);
    EXPECT_EQ(config.getConfigFilePath(), testConfigFile);
    EXPECT_EQ(config.getVersion(), "1.2.3"); // 来自testConfigFile，不是envConfigFile
}

// 测试11: 根据ID获取公式配置
TEST_F(TdxpyConfigManagerTest, GetFormulaById_Valid) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    // 获取存在的公式
    const tdxpy::config::FormulaConfig* formula = config.getFormulaById(2);
    ASSERT_NE(formula, nullptr);
    EXPECT_EQ(formula->name, "TDXPY_RSI");
    EXPECT_EQ(formula->description, "相对强弱指数");
    EXPECT_EQ(formula->function, "calculate_rsi");
    
    // 获取另一个公式
    formula = config.getFormulaById(4);
    ASSERT_NE(formula, nullptr);
    EXPECT_EQ(formula->name, "TDXPY_BOLL");
}

// 测试12: 根据ID获取不存在的公式
TEST_F(TdxpyConfigManagerTest, GetFormulaById_NotFound) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    // 获取不存在的ID
    const tdxpy::config::FormulaConfig* formula = config.getFormulaById(999);
    EXPECT_EQ(formula, nullptr);
    
    formula = config.getFormulaById(0);
    EXPECT_EQ(formula, nullptr);
    
    formula = config.getFormulaById(-1);
    EXPECT_EQ(formula, nullptr);
}

// 测试13: 根据名称获取公式配置
TEST_F(TdxpyConfigManagerTest, GetFormulaByName_Valid) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    // 获取存在的公式
    const tdxpy::config::FormulaConfig* formula = config.getFormulaByName("TDXPY_MACD");
    ASSERT_NE(formula, nullptr);
    EXPECT_EQ(formula->id, 3);
    EXPECT_EQ(formula->moduleName, "macd_indicator");
    EXPECT_EQ(formula->userParams, "12,26,9");
}

// 测试14: 根据名称获取不存在的公式
TEST_F(TdxpyConfigManagerTest, GetFormulaByName_NotFound) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    const tdxpy::config::FormulaConfig* formula = config.getFormulaByName("NON_EXISTENT");
    EXPECT_EQ(formula, nullptr);
    
    formula = config.getFormulaByName("");
    EXPECT_EQ(formula, nullptr);
}

// 测试15: 检查公式是否存在
TEST_F(TdxpyConfigManagerTest, ContainsFormula) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    EXPECT_TRUE(config.containsFormula("TDXPY_MA"));
    EXPECT_TRUE(config.containsFormula("TDXPY_RSI"));
    EXPECT_TRUE(config.containsFormula("TDXPY_MACD"));
    EXPECT_TRUE(config.containsFormula("TDXPY_BOLL"));
    EXPECT_FALSE(config.containsFormula("TDXPY_NONE"));
    EXPECT_FALSE(config.containsFormula(""));
}

// 测试16: 获取所有公式名称
TEST_F(TdxpyConfigManagerTest, GetFormulaNames) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    std::vector<std::string> names = config.getFormulaNames();
    EXPECT_EQ(names.size(), 4);
    
    // 验证所有名称都存在
    EXPECT_NE(std::find(names.begin(), names.end(), "TDXPY_MA"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "TDXPY_RSI"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "TDXPY_MACD"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "TDXPY_BOLL"), names.end());
    
    // 验证顺序（应该与配置文件中的顺序一致）
    EXPECT_EQ(names[0], "TDXPY_MA");
    EXPECT_EQ(names[1], "TDXPY_RSI");
    EXPECT_EQ(names[2], "TDXPY_MACD");
    EXPECT_EQ(names[3], "TDXPY_BOLL");
}

// 测试17: 验证配置有效性
TEST_F(TdxpyConfigManagerTest, Validate_ValidConfig) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    bool isValid = config.validate();
    EXPECT_TRUE(isValid);
}

// 测试18: 验证未加载的配置
TEST_F(TdxpyConfigManagerTest, Validate_NotLoaded) {
    tdxpy::config::ConfigManager config;
    
    bool isValid = config.validate();
    EXPECT_FALSE(isValid);
}

// 测试19: 验证缺少必需字段的配置
TEST_F(TdxpyConfigManagerTest, Validate_MissingRequiredFields) {
    // 创建缺少必需字段的配置
    std::string incompleteFile = testDir + "/incomplete_config.json";
    std::ofstream file(incompleteFile);
    file << R"({
    "tdxpy_formulas": {
        "version": "1.0.0"
        // 缺少description和author
    },
    "python_config": {
        // 缺少必需字段
    },
    "logging_config": {},
    "formula_mappings": []
})";
    file.close();
    
    tdxpy::config::ConfigManager config(incompleteFile);
    bool loaded = config.load();
    EXPECT_FALSE(loaded); // 即使缺少字段，load失败
    
    bool isValid = config.validate();
    EXPECT_FALSE(isValid); // 验证应该失败
}

// 测试20: 生成默认配置
TEST_F(TdxpyConfigManagerTest, GenerateDefaultConfig) {
    std::string defaultConfig = tdxpy::config::ConfigManager::generateDefaultConfig();
    
    EXPECT_FALSE(defaultConfig.empty());
    
    // 验证包含关键字段
    EXPECT_NE(defaultConfig.find("tdxpy_formulas"), std::string::npos);
    EXPECT_NE(defaultConfig.find("python_config"), std::string::npos);
    EXPECT_NE(defaultConfig.find("formula_mappings"), std::string::npos);
    EXPECT_NE(defaultConfig.find("metadata"), std::string::npos);
    
    // 验证包含默认值
    EXPECT_NE(defaultConfig.find("0.1.0"), std::string::npos);
    EXPECT_NE(defaultConfig.find("TDXPY_MA"), std::string::npos);
    
    // 应该是有效的JSON
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;
    
    bool parsingSuccessful = reader->parse(
        defaultConfig.c_str(),
        defaultConfig.c_str() + defaultConfig.length(),
        &root, &errors);
    
    EXPECT_TRUE(parsingSuccessful) << u8"JSON解析错误: " << errors;
}

// 测试21: 保存配置到文件
TEST_F(TdxpyConfigManagerTest, SaveToFile) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    // 保存到新文件
    std::string saveFile = testDir + "/saved_config.json";
    bool saved = config.saveToFile(saveFile);
    
    EXPECT_TRUE(saved);
    EXPECT_TRUE(fs::exists(saveFile));
    
    // 验证保存的文件可以加载
    tdxpy::config::ConfigManager loadedConfig(saveFile);
    bool reloaded = loadedConfig.load();
    EXPECT_TRUE(reloaded);
    
    // 验证内容一致
    EXPECT_EQ(loadedConfig.getVersion(), config.getVersion());
    EXPECT_EQ(loadedConfig.getFormulaMappings().size(), config.getFormulaMappings().size());
}

// 测试22: 保存未加载的配置
TEST_F(TdxpyConfigManagerTest, SaveToFile_NotLoaded) {
    tdxpy::config::ConfigManager config;
    
    std::string saveFile = testDir + "/save_fail.json";
    bool saved = config.saveToFile(saveFile);
    
    EXPECT_FALSE(saved);
    EXPECT_FALSE(fs::exists(saveFile));
}

// 测试23: 保存到无效路径
TEST_F(TdxpyConfigManagerTest, SaveToFile_InvalidPath) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    // 尝试保存到不存在的目录
    std::string invalidPath = "/nonexistent/directory/config.json";
    bool saved = config.saveToFile(invalidPath);
    
    EXPECT_FALSE(saved);
}

// 测试24: 打印配置摘要
TEST_F(TdxpyConfigManagerTest, PrintSummary) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    // 重定向stdout进行验证
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    
    config.printSummary();
    
    std::cout.rdbuf(old); // 恢复
    
    std::string output = buffer.str();
    
    // 验证输出包含关键信息
    EXPECT_NE(output.find(u8"=== TDXPY配置摘要 ==="), std::string::npos);
    EXPECT_NE(output.find(u8"版本:"), std::string::npos);
    EXPECT_NE(output.find("1.2.3"), std::string::npos);
    EXPECT_NE(output.find("4"), std::string::npos);
}

// 测试25: 打印未加载的配置摘要
TEST_F(TdxpyConfigManagerTest, PrintSummary_NotLoaded) {
    tdxpy::config::ConfigManager config;
    
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    
    config.printSummary();
    
    std::cout.rdbuf(old);
    
    std::string output = buffer.str();
    EXPECT_NE(output.find(u8"配置未加载"), std::string::npos);
}

// 测试26: 打印详细配置信息
TEST_F(TdxpyConfigManagerTest, PrintDetailedInfo) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    
    config.printDetailedInfo();
    
    std::cout.rdbuf(old);
    
    std::string output = buffer.str();
    
    // 验证各个部分都存在
    EXPECT_NE(output.find(u8"--- 基础配置 ---"), std::string::npos);
    EXPECT_NE(output.find(u8"--- Python配置 ---"), std::string::npos);
    EXPECT_NE(output.find(u8"--- 状态信息 ---"), std::string::npos);

    // 验证关键数据
    EXPECT_NE(output.find("TDXPY_MA"), std::string::npos);
    EXPECT_NE(output.find("移动平均线测试"), std::string::npos);
    EXPECT_NE(output.find("ma_indicator"), std::string::npos);
}

// 测试27: 解析公式映射（有效性检查）
TEST_F(TdxpyConfigManagerTest, ParseFormulaMapping_Valid) {
    Json::Value mapping;
    mapping["name"] = "TDXPY_TEST";
    mapping["description"] = "测试指标";
    mapping["path"] = "./test/";
    mapping["module_name"] = "test_module";
    mapping["function"] = "test_function";
    mapping["id"] = 100;
    mapping["user_params"] = "param1,param2";
    
    tdxpy::config::ConfigManager config;
    bool result = config.load(testConfigFile); // 先加载一个有效配置
    
    // 注意：parseFormulaMapping是私有方法，我们通过间接方式测试
    // 这里我们验证配置是否正确解析了包含这些字段的公式
    const auto* formula = config.getFormulaByName("TDXPY_MA");
    ASSERT_NE(formula, nullptr);
    
    // 验证解析的字段
    EXPECT_FALSE(formula->name.empty());
    EXPECT_FALSE(formula->function.empty());
    EXPECT_GT(formula->id, 0);
}

// 测试28: 公式映射ID唯一性检查
TEST_F(TdxpyConfigManagerTest, FormulaMapping_UniqueIds) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    const auto& formulas = config.getFormulaMappings();
    
    // 收集所有ID
    std::set<int> ids;
    for (const auto& formula : formulas) {
        ids.insert(formula.id);
    }
    
    // 验证所有ID都是唯一的
    EXPECT_EQ(ids.size(), formulas.size());
    
    // 验证ID都是正数
    for (int id : ids) {
        EXPECT_GT(id, 0);
    }
}

// 测试29: 公式名称前缀检查
TEST_F(TdxpyConfigManagerTest, FormulaName_PrefixCheck) {
    tdxpy::config::ConfigManager config(testConfigFile);
    bool loaded = config.load();
    ASSERT_TRUE(loaded);
    
    const auto& formulas = config.getFormulaMappings();
    
    // 验证所有公式名称都以TDXPY_开头
    for (const auto& formula : formulas) {
        EXPECT_EQ(formula.name.substr(0, 6), "TDXPY_");
    }
}


// 测试30: 空搜索路径处理
TEST_F(TdxpyConfigManagerTest, EmptySearchPaths) {
    // 创建没有搜索路径的配置
    std::string noPathsFile = testDir + "/no_paths_config.json";
    std::ofstream file(noPathsFile);
    file << R"({
    "tdxpy_formulas": {
        "version": "1.0.0",
        "description": "无搜索路径测试",
        "last_modified": "2026-01-12",
        "author": "测试",
        "license": "MIT"
    },
    "python_config": {
        "python_home": "./python/",
        "python_executable": "./python/python.exe",
        "python_formulas_home": "./formulas/",
        "enable_debug": false,
        "search_paths": []
    },
    "logging_config": {
        "log_file": "./test.log",
        "log_level": "INFO"
    },
    "formula_mappings": []
})";
    file.close();
    
    tdxpy::config::ConfigManager config(noPathsFile);
    bool loaded = config.load();
    
    EXPECT_TRUE(loaded);
    
    // 验证搜索路径为空
    const auto& searchPaths = config.getPythonSearchPaths();
    EXPECT_TRUE(searchPaths.empty());
}

// 测试31: 配置路径解析（相对路径转绝对路径）
TEST_F(TdxpyConfigManagerTest, ConfigPath_Resolution) {
    // 使用相对路径
    tdxpy::config::ConfigManager config("./" + testConfigFile);
    bool loaded = config.load();
    
    EXPECT_TRUE(loaded);
    // 注意：这里不验证路径是否转换为绝对路径，因为ConfigManager可能保持原样
}

// 测试32: 大量公式映射
TEST_F(TdxpyConfigManagerTest, LargeNumberOfFormulaMappings) {
    // 创建包含大量公式的配置文件
    std::string largeConfigFile = testDir + "/large_config.json";
    std::ofstream file(largeConfigFile);
    
    file << R"({
    "tdxpy_formulas": {
        "version": "1.0.0",
        "description": "大量公式测试",
        "last_modified": "2026-01-12",
        "author": "测试",
        "license": "MIT"
    },
    "python_config": {
        "python_home": "./python/",
        "python_executable": "./python/python.exe",
        "python_formulas_home": "./formulas/",
        "enable_debug": false,
        "search_paths": ["./"]
    },
    "logging_config": {
        "log_file": "./test.log",
        "log_level": "INFO"
    },
    "formula_mappings": [)";
    
    // 生成100个公式
    for (int i = 1; i <= 100; ++i) {
        file << R"(
        {
            "name": "TDXPY_FORMULA_)" << i << R"(",
            "description": "公式 )" << i << R"(",
            "path": "./formulas/",
            "module_name": "formula_module_)" << i << R"(",
            "function": "calculate_)" << i << R"(",
            "id": )" << i << R"(,
            "user_params": ")"
               << (i * 5) << "," << (i * 10) << R"("
        })";
        
        if (i < 100) {
            file << ",";
        }
    }
    
    file << R"(
    ]
})";
    file.close();
    
    // 加载配置
    tdxpy::config::ConfigManager config(largeConfigFile);
    auto start = std::chrono::high_resolution_clock::now();
    
    bool loaded = config.load();
    auto end = std::chrono::high_resolution_clock::now();
    
    EXPECT_TRUE(loaded);
    
    // 验证公式数量
    const auto& formulas = config.getFormulaMappings();
    EXPECT_EQ(formulas.size(), 100);
    
    // 验证性能（不应超过1秒）
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 1000) << "加载100个公式耗时: " << duration.count() << "ms";
    
    // 验证特定公式
    const auto* formula = config.getFormulaById(50);
    ASSERT_NE(formula, nullptr);
    EXPECT_EQ(formula->name, "TDXPY_FORMULA_50");
    EXPECT_EQ(formula->userParams, "250,500");
}

// 测试33: 配置验证边缘情况
TEST_F(TdxpyConfigManagerTest, Validate_EdgeCases) {
    // 测试各种边缘情况
    std::vector<std::pair<std::string, bool>> testCases = {
        // 缺少python_home
        {R"({
            "tdxpy_formulas": {"version":"1.0","description":"test","author":"test"},
            "python_config": {"python_executable":"./python.exe"},
            "logging_config": {"log_file":"./test.log","log_level":"INFO"},
            "formula_mappings": []
        })", false},
        
        // 缺少python_executable
        {R"({
            "tdxpy_formulas": {"version":"1.0","description":"test","author":"test"},
            "python_config": {"python_home":"./python/"},
            "logging_config": {"log_file":"./test.log","log_level":"INFO"},
            "formula_mappings": []
        })", false},
        
        // 有效的最小配置
        {R"({
            "tdxpy_formulas": {"version":"1.0","description":"test","author":"test"},
            "python_config": {
                "python_home":"./python/",
                "python_executable":"./python/python.exe"
            },
            "logging_config": {"log_file":"./test.log","log_level":"INFO"},
            "formula_mappings": []
        })", true},
        
        // 公式映射为空数组
        {R"({
            "tdxpy_formulas": {"version":"1.0","description":"test","author":"test"},
            "python_config": {
                "python_home":"./python/",
                "python_executable":"./python/python.exe"
            },
            "logging_config": {"log_file":"./test.log","log_level":"INFO"},
            "formula_mappings": []
        })", true},
    };
    
    for (size_t i = 0; i < testCases.size(); ++i) {
        const auto& [configJson, expectedValid] = testCases[i];
        
        std::string testFile = testDir + "/edge_case_" + std::to_string(i) + ".json";
        std::ofstream file(testFile);
        file << configJson;
        file.close();
        
        tdxpy::config::ConfigManager config(testFile);
        bool loaded = config.load();
        ASSERT_TRUE(loaded) << "测试用例 " << i << " 加载失败";
        
        bool isValid = config.validate();
        EXPECT_EQ(isValid, expectedValid) << "测试用例 " << i << " 验证结果不符合预期";
    }
}

// 测试34: JSON结构验证
TEST_F(TdxpyConfigManagerTest, ValidateJsonStructure) {
    // 这个测试验证私有方法validateJsonStructure的逻辑
    // 我们通过加载无效配置来间接测试
    
    // 缺少必需字段
    std::string missingFields = testDir + "/missing_fields.json";
    std::ofstream file1(missingFields);
    file1 << R"({
    "tdxpy_formulas": {
        "version": "1.0.0"
    }
    // 缺少其他必需字段
})";
    file1.close();
    
    tdxpy::config::ConfigManager config1(missingFields);
    bool loaded1 = config1.load();
    EXPECT_FALSE(loaded1);
    
    // formula_mappings不是数组
    std::string invalidMappings = testDir + "/invalid_mappings.json";
    std::ofstream file2(invalidMappings);
    file2 << R"({
    "tdxpy_formulas": {"version":"1.0","description":"test","author":"test"},
    "python_config": {"python_home":"./","python_executable":"./python.exe"},
    "logging_config": {"log_file":"./test.log","log_level":"INFO"},
    "formula_mappings": "not an array"
})";
    file2.close();
    
    tdxpy::config::ConfigManager config2(invalidMappings);
    bool loaded2 = config2.load();
    EXPECT_FALSE(loaded2);
}

// // 主函数
// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
    
//     // 设置UTF-8控制台输出（Windows需要）
//     #ifdef _WIN32
//     SetConsoleOutputCP(CP_UTF8);
//     #endif
    
//     return RUN_ALL_TESTS();
// }