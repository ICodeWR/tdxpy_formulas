/**
 * @file        tdxpy_env_var.c
 * @brief       通达信Python DLL环境变量管理器实现
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


#include <windows.h>
#include <vector>
#include <algorithm>
#include <memory>
#include "tdxpy_env_var.h"

namespace tdxpy {
namespace utils {

// 常量定义
constexpr DWORD MAX_ENV_VAR_SIZE = 32767;
constexpr wchar_t PATH_ENV_VAR[] = L"PATH";
constexpr wchar_t PATH_SEPARATOR = L';';

std::optional<std::string> EnvVarManager::get(const std::string& name) {
    auto wideName = utf8ToWide(name);
    if (!wideName) {
        return std::nullopt;
    }
    
    auto wideValue = getW(*wideName);
    if (!wideValue) {
        return std::nullopt;
    }
    
    return wideToUtf8(*wideValue);
}

std::optional<std::wstring> EnvVarManager::getW(const std::wstring& name) {
    if (name.empty()) {
        return std::nullopt;
    }
    
    // 获取所需缓冲区大小
    DWORD requiredSize = safeGetEnvVar(name.c_str(), nullptr, 0);
    if (requiredSize == 0) {
        return std::nullopt;
    }
    
    // 分配缓冲区
    std::vector<wchar_t> buffer(requiredSize);
    
    // 获取值
    DWORD actualSize = safeGetEnvVar(name.c_str(), buffer.data(), requiredSize);
    if (actualSize == 0 || actualSize >= requiredSize) {
        return std::nullopt;
    }
    
    return std::wstring(buffer.data(), actualSize);
}

bool EnvVarManager::set(const std::string& name, const std::string& value) {
    auto wideName = utf8ToWide(name);
    if (!wideName) {
        return false;
    }
    
    std::optional<std::wstring> wideValue;
    if (!value.empty()) {
        wideValue = utf8ToWide(value);
        if (!wideValue) {
            return false;
        }
    }
    
    return setW(*wideName, wideValue ? *wideValue : L"");
}

bool EnvVarManager::setW(const std::wstring& name, const std::wstring& value) {
    if (name.empty()) {
        return false;
    }
    
    return SetEnvironmentVariableW(name.c_str(), 
                                  value.empty() ? nullptr : value.c_str()) != FALSE;
}

bool EnvVarManager::remove(const std::string& name) {
    auto wideName = utf8ToWide(name);
    if (!wideName) {
        return false;
    }
    
    return SetEnvironmentVariableW(wideName->c_str(), nullptr) != FALSE;
}

bool EnvVarManager::exists(const std::string& name) {
    auto wideName = utf8ToWide(name);
    if (!wideName) {
        return false;
    }
    
    return GetEnvironmentVariableW(wideName->c_str(), nullptr, 0) != 0;
}

std::vector<std::pair<std::string, std::string>> EnvVarManager::getAll() {
    std::vector<std::pair<std::string, std::string>> result;
    
    // Windows环境块格式：每个变量以null结尾，整个块以两个null结尾
    wchar_t* envBlock = GetEnvironmentStringsW();
    if (!envBlock) {
        return result;
    }
    
    wchar_t* current = envBlock;
    while (*current != L'\0') {
        std::wstring envVar(current);
        
        // 分割变量名和值
        size_t equalPos = envVar.find(L'=');
        if (equalPos != std::wstring::npos) {
            std::wstring name = envVar.substr(0, equalPos);
            std::wstring value = envVar.substr(equalPos + 1);
            
            auto utf8Name = wideToUtf8(name);
            auto utf8Value = wideToUtf8(value);
            
            if (utf8Name && utf8Value) {
                result.emplace_back(*utf8Name, *utf8Value);
            }
        }
        
        current += envVar.length() + 1;
    }
    
    FreeEnvironmentStringsW(envBlock);
    return result;
}

std::vector<std::pair<std::string, std::string>> EnvVarManager::backup() {
    return getAll();
}

bool EnvVarManager::restore(const std::vector<std::pair<std::string, std::string>>& backup) {
    bool success = true;
    
    for (const auto& [name, value] : backup) {
        if (!set(name, value)) {
            success = false;
        }
    }
    
    return success;
}

bool EnvVarManager::addToPath(const std::string& path, bool front) {
    if (path.empty()) {
        return false;
    }
    
    auto currentPath = get("PATH");
    if (!currentPath) {
        return false;
    }
    
    // 分割当前PATH
    auto paths = splitPath(*currentPath);
    
    // 检查是否已存在
    auto normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() != '\\') {
        normalizedPath += "\\";
    }
    
    auto it = std::remove_if(paths.begin(), paths.end(), 
        [&normalizedPath](const std::string& p) {
            std::string normalizedP = p;
            if (!normalizedP.empty() && normalizedP.back() != '\\') {
                normalizedP += "\\";
            }
            return _stricmp(normalizedP.c_str(), normalizedPath.c_str()) == 0;
        });
    paths.erase(it, paths.end());
    
    // 添加路径
    if (front) {
        paths.insert(paths.begin(), path);
    } else {
        paths.push_back(path);
    }
    
    // 合并并设置
    std::string newPath = joinPath(paths);
    return set("PATH", newPath);
}

bool EnvVarManager::removeFromPath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    auto currentPath = get("PATH");
    if (!currentPath) {
        return false;
    }
    
    auto paths = splitPath(*currentPath);
    
    // 移除路径
    auto normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() != '\\') {
        normalizedPath += "\\";
    }
    
    auto it = std::remove_if(paths.begin(), paths.end(), 
        [&normalizedPath](const std::string& p) {
            std::string normalizedP = p;
            if (!normalizedP.empty() && normalizedP.back() != '\\') {
                normalizedP += "\\";
            }
            return _stricmp(normalizedP.c_str(), normalizedPath.c_str()) == 0;
        });
    
    if (it == paths.end()) {
        return true; // 路径不存在，不需要修改
    }
    
    paths.erase(it, paths.end());
    
    // 设置新的PATH
    std::string newPath = joinPath(paths);
    return set("PATH", newPath);
}

bool EnvVarManager::isInPath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    auto currentPath = get("PATH");
    if (!currentPath) {
        return false;
    }
    
    auto paths = splitPath(*currentPath);
    
    std::string normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() != '\\') {
        normalizedPath += "\\";
    }
    
    for (const auto& p : paths) {
        std::string normalizedP = p;
        if (!normalizedP.empty() && normalizedP.back() != '\\') {
            normalizedP += "\\";
        }
        
        if (_stricmp(normalizedP.c_str(), normalizedPath.c_str()) == 0) {
            return true;
        }
    }
    
    return false;
}

std::optional<std::wstring> EnvVarManager::utf8ToWide(const std::string& utf8Str) {
    if (utf8Str.empty()) {
        return L"";
    }
    
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, 
                                          utf8Str.c_str(), -1, 
                                          nullptr, 0);
    if (requiredSize <= 0) {
        return std::nullopt;
    }
    
    std::vector<wchar_t> buffer(requiredSize);
    int actualSize = MultiByteToWideChar(CP_UTF8, 0, 
                                        utf8Str.c_str(), -1, 
                                        buffer.data(), requiredSize);
    
    if (actualSize <= 0) {
        return std::nullopt;
    }
    
    return std::wstring(buffer.data(), actualSize - 1);
}

std::optional<std::string> EnvVarManager::wideToUtf8(const std::wstring& wideStr) {
    if (wideStr.empty()) {
        return "";
    }
    
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, 
                                          wideStr.c_str(), -1, 
                                          nullptr, 0, 
                                          nullptr, nullptr);
    if (requiredSize <= 0) {
        return std::nullopt;
    }
    
    std::vector<char> buffer(requiredSize);
    int actualSize = WideCharToMultiByte(CP_UTF8, 0, 
                                        wideStr.c_str(), -1, 
                                        buffer.data(), requiredSize, 
                                        nullptr, nullptr);
    
    if (actualSize <= 0) {
        return std::nullopt;
    }
    
    return std::string(buffer.data(), actualSize - 1);
}

DWORD EnvVarManager::safeGetEnvVar(const wchar_t* name, wchar_t* buffer, DWORD bufferSize) {
    if (!name) {
        return 0;
    }
    
    return GetEnvironmentVariableW(name, buffer, bufferSize);
}

std::vector<std::string> EnvVarManager::splitPath(const std::string& pathStr) {
    std::vector<std::string> result;
    
    if (pathStr.empty()) {
        return result;
    }
    
    size_t start = 0;
    size_t end = pathStr.find(';');
    
    while (end != std::string::npos) {
        std::string path = pathStr.substr(start, end - start);
        if (!path.empty()) {
            result.push_back(path);
        }
        start = end + 1;
        end = pathStr.find(';', start);
    }
    
    // 添加最后一个路径
    std::string lastPath = pathStr.substr(start);
    if (!lastPath.empty()) {
        result.push_back(lastPath);
    }
    
    return result;
}

std::string EnvVarManager::joinPath(const std::vector<std::string>& paths) {
    std::string result;
    
    for (size_t i = 0; i < paths.size(); ++i) {
        if (!paths[i].empty()) {
            result += paths[i];
            if (i != paths.size() - 1) {
                result += ";";
            }
        }
    }
    
    return result;
}

} // namespace utils
} // namespace tdxpy