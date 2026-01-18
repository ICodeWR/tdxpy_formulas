#pragma once

// 精简Windows头文件
#define WIN32_LEAN_AND_MEAN
// #define NOMINMAX
#define STRICT
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOPROXYSTUB

// Windows头文件
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>

// C运行时
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cassert>

// C++标准库
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <stack>
#include <deque>
#include <array>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <utility>
#include <tuple>
#include <type_traits>
#include <limits>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <chrono>
#include <random>

// STL流
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

// 异常处理
#include <exception>
#include <stdexcept>
#include <system_error>

// 项目特定宏
#ifdef MINIMALDLL_EXPORTS
#define MINIMALDLL_API __declspec(dllexport)
#else
#define MINIMALDLL_API __declspec(dllimport)
#endif

// 常用类型别名
using std::string;
using std::wstring;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;

// 常用常量
constexpr DWORD DEFAULT_TIMEOUT = 5000;
constexpr size_t MAX_PATH_SIZE = 32767;

// 调试支持
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#ifdef _CRTDBG_MAP_ALLOC
#define new DEBUG_NEW
#endif
#endif