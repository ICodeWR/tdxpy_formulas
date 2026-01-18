# 贡献指南

欢迎参与 **tdxpy_formulas** 项目！本指南将帮助您了解如何为项目做出贡献。

## 目录
- [行为准则](#行为准则)
- [快速开始](#快速开始)
- [开发环境配置](#开发环境配置)
- [代码规范](#代码规范)
- [提交规范](#提交规范)
- [测试要求](#测试要求)
- [文档要求](#文档要求)
- [问题报告与功能请求](#问题报告与功能请求)
- [Pull Request 流程](#pull-request-流程)
- [许可证](#许可证)
- [联系方式](#联系方式)

## 行为准则

参与本项目时，请遵守我们的[行为准则](CODE_OF_CONDUCT.md)。简要概括：

- **尊重他人**：无论技术水平高低，尊重每一位贡献者
- **建设性反馈**：提供具体、有建设性的反馈
- **包容性**：欢迎不同背景、经验的贡献者
- **专业精神**：保持专业、礼貌的交流态度

如有违反行为准则的情况，请发送邮件至：3892493481@qq.com

## 快速开始

如果你是第一次参与开源项目，建议从以下简单任务开始：

1. **修复文档错别字** - 在 README.md 或其他文档中修正错误
2. **改进代码注释** - 让代码注释更清晰易懂
3. **添加测试用例** - 为现有功能补充测试
4. **解决标记为 "good first issue" 的问题**

### 第一步：Fork 项目
1. 访问 [项目主页](https://gitee.com/icodewr/tdxpy_formulas)
2. 点击右上角的 "Fork" 按钮
3. 克隆你的 Fork 到本地：
```bash
git clone git@gitee.com:icodewr/tdxpy_formulas.git
cd tdxpy_formulas
```

### 第二步：设置上游仓库并创建分支
```bash
# 添加上游仓库
git remote add upstream git@gitee.com:icodewr/tdxpy_formulas.git

# 同步最新代码
git fetch upstream

# 创建功能分支
git checkout -b feature/your-feature-name upstream/main
```

### 第三步：开始贡献
请遵循后续章节的开发指南。完成后提交 Pull Request。

## 开发环境配置

### 必需工具
- **Visual Studio 2022**（含C++桌面开发工作负载）
- **CMake 3.15+**（建议3.20+）
- **Git**
- **Python 3.14.2 32位**（已包含在项目中）

### 安装方式选择

#### 方式一：使用包管理器
```powershell
# 使用 Chocolatey（Windows）
choco install cmake git

# 或使用 Scoop（Windows）
scoop install cmake git

# 或使用 Homebrew（macOS，仅用于开发）
brew install cmake git
```

#### 方式二：手动安装
1. 下载并安装 [Visual Studio 2022 Community](https://visualstudio.microsoft.com/)
2. 下载 [CMake](https://cmake.org/download/)
3. 下载 [Git](https://git-scm.com/download/)
4. Python 3.14.2 32位已包含在项目中，无需额外安装

### 配置与构建
```bash
# 配置项目（32位版本）
cmake -B build -A Win32

# 编译项目
cmake --build build --config Release

```

## 代码规范

### C++ 代码规范

#### 命名规范
```cpp
// 类名：大驼峰
class ConfigManager;

// 函数名：小驼峰
void initializePythonEngine();

// 变量名：小驼峰
int dataLength;

// 常量：全大写，下划线分隔
const int MAX_DATA_LENGTH = 1000;
```

#### 代码风格

```cpp
// 使用大括号，即使只有一行
if (condition) {
    doSomething();
}

// 指针和引用类型靠近变量名
float* data;  // 正确
```

### Python 代码规范

遵循 PEP 8 标准：
```python
# 函数和变量：小写下划线分隔
def calculate_indicator(data: list, period: int) -> list:
    """计算技术指标
    
    Args:
        data: 输入数据列表
        period: 计算周期
        
    Returns:
        计算结果列表
    """
    if len(data) < period:
        raise ValueError(f"数据长度不足: {len(data)} < {period}")
    
    return [sum(data[i:i+period])/period for i in range(len(data)-period+1)]

# 类名：大驼峰
class IndicatorCalculator:
    def __init__(self, name: str):
        self.name = name
```

## 提交规范

### 提交信息格式
```
类型(范围): 简要描述

详细描述（可选）

- 功能点1
- 功能点2

关联问题: #Issue编号
```

### 提交类型
| 类型 | 说明 | 示例 |
|------|------|------|
| `feat` | 新功能 | `feat(python): 添加MA指标支持` |
| `fix` | 错误修复 | `fix(config): 修复配置文件加载问题` |
| `docs` | 文档更新 | `docs(readme): 更新构建说明` |
| `style` | 代码格式 | `style(cpp): 统一代码格式` |
| `refactor` | 重构代码 | `refactor(engine): 重构Python引擎` |

### 提交示例
```bash
# 添加并提交变更
git add src/tdxpy_python_engine.cpp
git commit -m "feat(python): 添加自定义指标支持

- 添加新的Python指标接口
- 支持用户自定义参数

关联问题: #45"
```

## 文档要求

### 代码文档示例
```cpp
/**
 * @brief 初始化Python解释器
 * 
 * @param configPath 配置文件路径（可选）
 * @return 初始化结果，0表示成功
 * 
 * @note 此函数不是线程安全的
 * @warning 不要重复调用此函数
 */
int tdxpyPythonInitialize(const std::string& configPath = "");
```

### 用户文档
- 更新 README.md 中的相关部分
- 维护 docs/ 目录下的详细文档
- 更新 examples/ 中的示例代码

## 问题报告与功能请求

### 问题报告模板
```markdown
## 问题描述
清晰描述遇到的问题

## 重现步骤
1. 第一步
2. 第二步

## 环境信息
- 操作系统：Windows 10/11
- 通达信版本：7.xx
- 项目版本：0.1.0
```

### 功能请求模板
```markdown
## 功能描述
清晰描述想要的功能

## 使用场景
描述该功能的使用场景和解决的问题

## 建议实现
如果有实现建议，请描述
```

## Pull Request 流程

### 创建 PR 前检查清单
- [ ] 代码符合规范
- [ ] 运行所有测试并通过
- [ ] 更新相关文档
- [ ] 提交信息规范

### PR 模板
```markdown
## 变更类型
- [ ] 新功能
- [ ] 错误修复
- [ ] 代码重构
- [ ] 文档更新

## 变更描述
详细描述本次变更的内容

## 相关 Issue
关联的 Issue 编号：#123

## 测试验证
- [ ] 已通过所有现有测试
- [ ] 在本地环境测试通过
```

### PR 审查流程
1. **自动检查**：CI/CD流水线自动运行
2. **代码审查**：至少需要一位维护者审查
3. **功能测试**：在测试环境验证
4. **合并决策**：审查通过后合并

## 📄 许可证

通过向本项目提交代码，您同意：
1. 您的贡献将根据项目的 MIT 许可证授权
2. 您拥有提交代码的合法权利
3. 代码不侵犯任何第三方权利

## 📞 联系方式

### 核心维护者
- **码上工坊**：项目创始人和主要维护者

### 沟通渠道
1. **GitHub Issues**：问题报告和功能请求
2. **邮箱**：3892493481@qq.com
3. **GitHub Discussions**：技术讨论和问答


## 🙏 致谢

感谢所有为项目做出贡献的开发者！您的每一行代码、每一个Issue、每一次PR都让项目变得更好。

### 贡献者榜单
- [查看所有贡献者](https://github.com/ICodeWR/tdxpy_formulas/graphs/contributors)

---

**💪 开始你的第一次贡献吧！**

如果你不确定从哪里开始：
1. 查看标记为 "good first issue" 的问题
2. 改进文档或添加注释
3. 编写简单的测试用例

我们期待你的参与！🚀

---