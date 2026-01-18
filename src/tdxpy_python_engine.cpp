/**
 * @file        tdxpy_python_engine.cpp
 * @brief       C++调用Python引擎实现
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

#include "pch.h"
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include "json/json.h"
#include "tdxpy_python_engine.h"
#include "tdxpy_config_manager.h"
#include "tdxpy_logger.h"
#include "tdxpy_env_var.h"

// ==================================================================
// 版本兼容性宏
// ==================================================================

// 检测Python版本
#if PY_VERSION_HEX >= 0x030E0000  // 3.14.0
    #define TDXPY_PYTHON_314_PLUS 1
#elif PY_VERSION_HEX >= 0x03090000  // 3.9.0
    #define TDXPY_PYTHON_39_PLUS 1
#elif PY_VERSION_HEX >= 0x03070000  // 3.7.0
    #define TDXPY_PYTHON_37_PLUS 1
#endif

// ==================================================================
// 线程安全辅助类
// ==================================================================

/**
 * @brief Python GIL管理类（RAII）
 */
class PyGILLocker {
private:
    PyGILState_STATE gstate;
    bool bAcquired;
    
public:
    PyGILLocker() : bAcquired(false) {
        if (Py_IsInitialized()) {
            gstate = PyGILState_Ensure();
            bAcquired = true;
        }
    }
    
    ~PyGILLocker() {
        if (bAcquired) {
            PyGILState_Release(gstate);
        }
    }
    
    // 禁止拷贝
    PyGILLocker(const PyGILLocker&) = delete;
    PyGILLocker& operator=(const PyGILLocker&) = delete;
    
    bool isAcquired() const { return bAcquired; }
};

// ==================================================================
// 全局变量
// ==================================================================

// 全局配置管理器
static tdxpy::config::ConfigManager g_tdxpyConfig("");

// 线程安全变量
static std::atomic<int> g_tdxpyLastFunctionId{0};           // 原子操作保护的最后函数ID
static std::shared_mutex g_configMutex;                     // 配置读写锁
static std::atomic<bool> g_pythonInitialized{false};        // Python初始化状态
static std::mutex g_initMutex;                              // Python初始化互斥锁
// ==================================================================
// 辅助函数
// ==================================================================

/**
 * @brief 安全的Python错误处理
 */
static void SafePyErrPrint() {
    if (PyErr_Occurred()) {
        PyGILLocker gil;
        PyErr_Print();
        PyErr_Clear();  // 清除错误状态
    }
}

/**
 * @brief 获取Python版本字符串
 */
static std::string GetPythonVersionString() {
    return std::to_string(PY_MAJOR_VERSION) + "." +
           std::to_string(PY_MINOR_VERSION) + "." +
           std::to_string(PY_MICRO_VERSION);
}

/**
 * @brief 设置虚拟环境相关的环境变量
 */ 
static bool setupVenvEnvironment(std::string& refVenvPath) {

    bool success = true;
    
    // 环境变量键名
    static constexpr const char* ENV_VIRTUAL_ENV = "VIRTUAL_ENV";
    static constexpr const char* ENV_PYTHONHOME = "PYTHONHOME";
    static constexpr const char* ENV_PATH = "PATH";

    // 获取虚拟环境路径
    if (refVenvPath.empty()) {
        return false;
    }   

    // 设置VIRTUAL_ENV
    if (!tdxpy::utils::EnvVarManager::set(ENV_VIRTUAL_ENV, refVenvPath)) {
        success = false;
    }
    
    // 设置PYTHONHOME为空，强制使用虚拟环境
    if (!tdxpy::utils::EnvVarManager::set(ENV_PYTHONHOME, "")) {
        success = false;
    }

    // 构建Python可执行文件路径
    std::string scriptsDir = refVenvPath + "\\Scripts";

    // 添加Scripts目录到PATH
    if (!tdxpy::utils::EnvVarManager::addToPath(scriptsDir, true)) {
        success = false;
    }
    
    return success;
}
// ==================================================================
// 主要函数实现
// ==================================================================

/**
 * @brief 初始化Python引擎
 * @return 成功返回0，失败返回1
 */
int tdxpyPythonInitialize()
{
    // 检查是否已初始化，第一次检查：快速路径（无锁）
    if (g_pythonInitialized.load(std::memory_order_acquire)) {
        TDXPY_LOG_WARNING(u8"Python已初始化，跳过重复初始化");
        return 0;
    }
    
    // 获取初始化互斥锁
    std::lock_guard<std::mutex> lock(g_initMutex);

    // 再次检查（双检锁模式），第二重检查：持有锁检查
    if (g_pythonInitialized.load(std::memory_order_relaxed)) {
        return 0;
    }


    
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    PyConfig pythonConfig;

    // 初始化日志系统(使用默认Log文件)
    if (!tdxpy::Logger::getInstance().initialize(TDXPY_DEFAULT_LOG_FILE, tdxpy::LogWarning))
    {
        std::cerr << "Failed to initialize logger" << std::endl;
        return 1;
    }

    // 配置文件名使用环境变量配置或者默认配置
    {
        std::unique_lock<std::shared_mutex> configLock(g_configMutex);
        if (!g_tdxpyConfig.load())
        {
            g_tdxpyConfig.generateDefaultConfig();
        }
    }

    // 获取配置的日志级别
    std::string logLevelStr;
    std::string logFile;
    {
        std::shared_lock<std::shared_mutex> configLock(g_configMutex);
        logLevelStr = g_tdxpyConfig.getLoggingLogLevel();
        logFile = g_tdxpyConfig.getLoggingLogFile();
    }
    
    tdxpy::LogLevel logLevel = tdxpy::Logger::getInstance().getLevelFromString(logLevelStr);

    // 重新设置日志系统(使用配置Log文件)
    if (!tdxpy::Logger::getInstance().initialize(logFile, logLevel))
    {
        std::cerr << "Failed to initialize logger" << std::endl;
        tdxpy::Logger::getInstance().initialize(TDXPY_DEFAULT_LOG_FILE, tdxpy::LogDebug);
    }
    TDXPY_LOG_INFO(u8"日志系统初始化成功，日志级别：" + logLevelStr);

    // Python 虚拟环境配置
    {
        std::shared_lock<std::shared_mutex> configLock(g_configMutex);
        std::string venvPath = g_tdxpyConfig.getPythonVenvHome();
        if (!venvPath.empty()) {
            if (setupVenvEnvironment(venvPath)) {
                TDXPY_LOG_INFO(u8"已设置Python虚拟环境相关环境变量");
            } else {
                TDXPY_LOG_WARNING(u8"设置Python虚拟环境相关环境变量失败");
            }
        } else {
            TDXPY_LOG_INFO(u8"未配置Python虚拟环境，使用系统默认Python环境");
        }
        TDXPY_LOG_DEBUG(u8"虚拟环境路径: " + venvPath);
    }
    
    // 显示Python版本信息
    std::string pyVersion = GetPythonVersionString();
    TDXPY_LOG_INFO(u8"Python版本: " + pyVersion);
    TDXPY_LOG_DEBUG(u8"开始初始化Python解释器");

    // 初始化Python解释器配置
    PyConfig_InitPythonConfig(&pythonConfig);

    // 1. 设置Python解释器相关路径
    std::string pythonExe, pythonHome;
    {
        std::shared_lock<std::shared_mutex> configLock(g_configMutex);
        pythonExe = g_tdxpyConfig.getPythonExecutableFile();
        pythonHome = g_tdxpyConfig.getPythonHomePath();
    }
    
    pythonConfig.executable = Py_DecodeLocale(pythonExe.c_str(), nullptr);
    pythonConfig.home = Py_DecodeLocale(pythonHome.c_str(), nullptr);
    pythonConfig.prefix = Py_DecodeLocale(pythonHome.c_str(), nullptr);
    pythonConfig.exec_prefix = Py_DecodeLocale(pythonHome.c_str(), nullptr);

    // 2. 设置Python库/模块搜索路径（关键路径）
    pythonConfig.module_search_paths_set = 1;
    PyStatus status;

    std::vector<std::string> pySearchPaths;
    {
        std::shared_lock<std::shared_mutex> configLock(g_configMutex);
        pySearchPaths = g_tdxpyConfig.getPythonSearchPaths();
    }
    
    for (const auto& path : pySearchPaths)
    {
        status = PyWideStringList_Append(&pythonConfig.module_search_paths, Py_DecodeLocale(path.c_str(), nullptr));
        if (PyStatus_Exception(status))
        {
            TDXPY_LOG_WARNING(u8"添加搜索路径出错：" + path);
        }
    }

    // 3. 设置为隔离模式，避免使用环境变量影响嵌入式Python
    pythonConfig.isolated = 1;

    // Python将：
    // 1. 忽略 PYTHONPATH
    // 2. 忽略 PYTHONHOME
    // 3. 忽略所有 PYTHON* 环境变量
    // 4. 只使用 config 中设置的路径
    pythonConfig.use_environment = 0;

    // 4. 初始化Python解释器
    status = Py_InitializeFromConfig(&pythonConfig);
    if (PyStatus_Exception(status))
    {
        TDXPY_LOG_ERROR(u8"Python解释器初始化失败");
        return 1;
    }

    // 5. Python 3.7+ 自动初始化线程支持，无需额外操作
    
    TDXPY_LOG_INFO(u8"Python解释器初始化成功!");
    
    // 6. 设置初始化标志
    g_pythonInitialized.store(true, std::memory_order_release);

    // 记录初始化完成
    TDXPY_LOG_DEBUG(u8"Python初始化完成，释放初始化锁");

    return 0;
}

/**
 * @brief 释放Python引擎资源
 * @return 始终返回1
 */
int tdxpyPythonDeinitialize(void)
{
    // 检查是否已初始化
    if (!g_pythonInitialized.load(std::memory_order_acquire)) {
        TDXPY_LOG_WARNING(u8"Python未初始化，无需释放");
        return 1;
    }
    
    // 获取初始化互斥锁
    std::lock_guard<std::mutex> lock(g_initMutex);

    // 再次检查：持有锁检查
    if (!g_pythonInitialized.load(std::memory_order_relaxed)) {
        return 1;
    }
    
    TDXPY_LOG_DEBUG(u8"开始释放Python解释器资源...");
    
    // 获取GIL以确保安全清理
    PyGILLocker gil;
    if (Py_IsInitialized()) {
        Py_Finalize();
        TDXPY_LOG_INFO(u8"Python解释器已关闭");
    } else {
        TDXPY_LOG_WARNING(u8"Python解释器未初始化，无需关闭");
    }
    
    // 清除初始化标志
    g_pythonInitialized.store(false, std::memory_order_release);
    TDXPY_LOG_DEBUG(u8"Python解释器已关闭");
    return 1;
}

/**
 * @brief 运行Python插件函数
 * @param functionId 功能ID号
 * @param dataLength 数据长度
 * @param pfOUT 输出数据数组
 * @param pfINa 第一组输入数据数组
 * @param pfINb 第二组输入数据数组
 * @param pfINc 第三组输入数据数组
 * @return 成功返回1，失败返回0
 */
int tdxpyRunPythonPlugin(int functionId, int dataLength,
    float* pfOUT, float* pfINa, float* pfINb, float* pfINc)
{
    // 0. 参数校验
    if (dataLength <= 0 || !pfOUT) {
        TDXPY_LOG_ERROR(u8"无效参数: dataLength=" + std::to_string(dataLength));
        return 0;
    }
    
    // 1. 线程安全的函数ID记录
    if (functionId) {
        g_tdxpyLastFunctionId.store(functionId, std::memory_order_release);
    }
    
    // 2. 检查Python是否已初始化
    if (!g_pythonInitialized.load(std::memory_order_acquire)) {
        TDXPY_LOG_ERROR(u8"Python解释器未初始化");
        return 0;
    }
    
    // 3. 获取配置信息（共享读锁）
    const tdxpy::config::FormulaConfig* pFormula = nullptr;
    {
        std::shared_lock<std::shared_mutex> configLock(g_configMutex);
        pFormula = g_tdxpyConfig.getFormulaById(functionId);
        if (pFormula == nullptr)
        {
            TDXPY_LOG_ERROR(u8"未找到公式，ID: " + std::to_string(functionId));
            return 0;
        }
    }
    
    // 4. 自动管理GIL
    PyGILLocker gilLocker;
    if (!gilLocker.isAcquired()) {
        TDXPY_LOG_ERROR(u8"无法获取GIL");
        return 0;
    }
    
    // 5. 局部变量
    int resultValue = 0;
    PyObject* pModule = nullptr;
    PyObject* pFunction = nullptr;
    PyObject* pArgs = nullptr;
    PyObject* pResult = nullptr;
    
    try {
        // 6. 模块导入和函数调用
        TDXPY_LOG_DEBUG(u8"加载模块: " + pFormula->moduleName);
        
        // 7. 添加模块路径
        if (!pFormula->path.empty())
        {
            std::string sysPathCmd = "import sys\n";
            sysPathCmd += "if '" + pFormula->path + "' not in sys.path:\n";
            sysPathCmd += "    sys.path.append('" + pFormula->path + "')\n";
            
            TDXPY_LOG_DEBUG(u8"设置模块路径: " + pFormula->path);
            PyRun_SimpleString(sysPathCmd.c_str());
        }
        
        // 8. 导入模块
        pModule = PyImport_ImportModule(pFormula->moduleName.c_str());
        if (!pModule)
        {
            SafePyErrPrint();
            TDXPY_LOG_ERROR(u8"加载模块失败: " + pFormula->moduleName);
            goto cleanup;
        }
        
        // 9. 调试功能的重加载
#if TDXPY_MODULE_RELOAD_ENABLED == 1
        if (functionId == 0)
        {
            PyObject* reloaded = PyImport_ReloadModule(pModule);
            if (!reloaded)
            {
                SafePyErrPrint();
                TDXPY_LOG_ERROR(u8"重新加载模块失败: " + pFormula->moduleName);
                Py_XDECREF(pModule);
                pModule = nullptr;
                goto cleanup;
            }
            Py_XDECREF(pModule);
            pModule = reloaded;
            resultValue = 1;
            goto cleanup;
        }
#endif // TDXPY_MODULE_RELOAD_ENABLED
        
        // 10. 获取Python函数
        pFunction = PyObject_GetAttrString(pModule, pFormula->function.c_str());
        if (!pFunction || !PyCallable_Check(pFunction))
        {
            SafePyErrPrint();
            TDXPY_LOG_ERROR(u8"加载函数失败: " + pFormula->function);
            goto cleanup;
        }
        
        // 11. 构造参数
        pArgs = PyTuple_New(6);
        if (!pArgs)
        {
            TDXPY_LOG_ERROR(u8"创建参数元组失败");
            goto cleanup;
        }
        
        // 12. 创建输入列表
        PyObject* pInputList1 = PyList_New(dataLength);
        PyObject* pInputList2 = PyList_New(dataLength);
        PyObject* pInputList3 = PyList_New(dataLength);
        
        if (!pInputList1 || !pInputList2 || !pInputList3)
        {
            Py_XDECREF(pInputList1);
            Py_XDECREF(pInputList2);
            Py_XDECREF(pInputList3);
            TDXPY_LOG_ERROR(u8"创建输入列表失败");
            goto cleanup;
        }
        
        // 13. 填充数据
        for (int i = 0; i < dataLength; i++)
        {
            double val1 = pfINa ? pfINa[i] : 0.0;
            double val2 = pfINb ? pfINb[i] : 0.0;
            double val3 = pfINc ? pfINc[i] : 0.0;
            
            PyObject* item1 = PyFloat_FromDouble(val1);
            PyObject* item2 = PyFloat_FromDouble(val2);
            PyObject* item3 = PyFloat_FromDouble(val3);
            
            if (item1) PyList_SetItem(pInputList1, i, item1);
            if (item2) PyList_SetItem(pInputList2, i, item2);
            if (item3) PyList_SetItem(pInputList3, i, item3);
        }
        
        // 14. 设置元组参数
        PyTuple_SetItem(pArgs, 0, PyLong_FromLong(functionId));
        PyTuple_SetItem(pArgs, 1, PyLong_FromLong(dataLength));
        PyTuple_SetItem(pArgs, 2, pInputList1);  // 转移所有权
        PyTuple_SetItem(pArgs, 3, pInputList2);
        PyTuple_SetItem(pArgs, 4, pInputList3);
        PyTuple_SetItem(pArgs, 5, PyUnicode_FromString(pFormula->userParams.c_str()));
        
        // 15. 调用Python函数
        TDXPY_LOG_DEBUG(u8"调用函数: " + pFormula->function);
        
        pResult = PyObject_CallObject(pFunction, pArgs);
        
        // 16. 处理返回结果
        if (pResult && PyList_Check(pResult))
        {
            Py_ssize_t listSize = PyList_Size(pResult);
            Py_ssize_t safeSize = (listSize < dataLength) ? listSize : dataLength;
            
            for (Py_ssize_t i = 0; i < safeSize; i++)
            {
                PyObject* pItem = PyList_GetItem(pResult, i);
                if (pItem && PyFloat_Check(pItem))
                {
                    pfOUT[i] = static_cast<float>(PyFloat_AsDouble(pItem));
                }
                else if (pItem && PyLong_Check(pItem))
                {
                    pfOUT[i] = static_cast<float>(PyLong_AsDouble(pItem));
                }
                else
                {
                    pfOUT[i] = 0.0f;
                }
            }
            
            // 填充剩余部分
            for (Py_ssize_t i = safeSize; i < dataLength; i++) {
                pfOUT[i] = 0.0f;
            }
            
            resultValue = 1;
            TDXPY_LOG_DEBUG(u8"函数调用成功，处理数据: " + std::to_string(safeSize) + u8"条");
        }
        else if (pResult && pResult == Py_None)
        {
            resultValue = 1;
            TDXPY_LOG_DEBUG(u8"函数返回None");
        }
        else
        {
            SafePyErrPrint();
            TDXPY_LOG_WARNING(u8"函数返回值不是列表类型或返回错误");
        }
    }
    catch (const std::exception& e)
    {
        TDXPY_LOG_ERROR(u8"C++异常: " + std::string(e.what()));
        SafePyErrPrint();
    }
    catch (...)
    {
        TDXPY_LOG_ERROR(u8"未知C++异常");
        SafePyErrPrint();
    }
    
cleanup:
    // 17. 清理Python对象
    Py_XDECREF(pResult);
    Py_XDECREF(pArgs);
    Py_XDECREF(pFunction);
    Py_XDECREF(pModule);
    
    // 18. GIL由PyGILLocker自动释放
    
    TDXPY_LOG_DEBUG(u8"函数调用结束，结果: " + std::to_string(resultValue));
    return resultValue;
}

/**
 * @brief 获取最后使用的函数ID
 * @return 最后使用的函数ID
 */
int tdxpyGetLastFunctionId()
{
    return g_tdxpyLastFunctionId.load(std::memory_order_acquire);
}

/**
 * @brief 检查Python是否已初始化
 * @return 已初始化返回true
 */
bool tdxpyIsPythonInitialized()
{
    return g_pythonInitialized.load(std::memory_order_acquire);
}

/**
 * @brief 重新加载配置
 * @return 成功返回true
 */
bool tdxpyReloadConfig()
{
    std::unique_lock<std::shared_mutex> configLock(g_configMutex);
    return g_tdxpyConfig.load();
}

/**
 * @brief 获取Python版本信息
 * @return Python版本字符串
 */
const char* tdxpyGetPythonVersion()
{
    static std::string version = GetPythonVersionString();
    return version.c_str();
}

/**
 * @brief 检查是否是Python 3.14或更高版本
 * @return 是3.14+返回true
 */
bool tdxpyIsPython314OrHigher()
{
#if TDXPY_PYTHON_314_PLUS
    return true;
#else
    return false;
#endif
}