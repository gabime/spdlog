# VSCode/Cursor配置文件详细指南

## 概述
本文档详细解释了Automation项目中`.vscode`目录下各个配置文件的作用、配置项说明和使用方法。

## 文件结构
```
.vscode/
├── launch.json      # 调试和运行配置
└── settings.json    # 项目设置和CMake配置
```

---

## launch.json - 调试和运行配置

### 文件作用
`launch.json` 是VSCode/Cursor的调试配置文件，定义了如何启动、调试和运行您的程序。它相当于Qt Creator中的"运行配置"。

### 配置结构说明

#### 1. 基本结构
```json
{
    "version": "0.2.0",           // 配置文件版本
    "configurations": [           // 配置数组，可包含多个配置
        {
            // 配置1
        },
        {
            // 配置2
        }
    ]
}
```

#### 2. Debug Automation (Qt Creator Style) 配置详解

```json
{
    "name": "Debug Automation (Qt Creator Style)",    // 配置名称，显示在调试面板中
    "type": "cppdbg",                                 // 调试器类型：C++调试器
    "request": "launch",                              // 请求类型：启动程序
    "program": "${workspaceFolder}/build/Windows/Debug/bin/AutomationGui_d.exe",  // 可执行文件路径
    "args": [],                                       // 命令行参数
    "stopAtEntry": false,                             // 是否在程序入口处停止
    "cwd": "${workspaceFolder}/build/Windows/Debug/bin",  // 工作目录
    "environment": [                                  // 环境变量设置
        {
            "name": "PATH",
            "value": "C:\\Qt\\6.9.1\\mingw_64\\bin;${workspaceFolder}\\build\\Windows\\Debug\\pluginspackage;${workspaceFolder}\\build\\Windows\\Release\\pluginspackage;${env:PATH}"
        },
        {
            "name": "QT_PLUGIN_PATH",
            "value": "C:\\Qt\\6.9.1\\mingw_64\\plugins"
        },
        {
            "name": "QT_QPA_PLATFORM_PLUGIN_PATH",
            "value": "C:\\Qt\\6.9.1\\mingw_64\\plugins\\platforms"
        }
    ],
    "externalConsole": false,                         // 是否使用外部控制台
    "MIMode": "gdb",                                  // 调试器接口模式
    "miDebuggerPath": "C:\\Qt\\Tools\\mingw1310_64\\bin\\gdb.exe",  // GDB调试器路径
    "setupCommands": [                                // GDB初始化命令
        {
            "description": "Enable pretty-printing for gdb",
            "text": "-enable-pretty-printing",
            "ignoreFailures": true
        }
    ],
    "preLaunchTask": "CMake: build"                   // 启动前执行的任务
}
```

#### 3. 关键配置项说明

##### 路径变量
- `${workspaceFolder}`: 工作区根目录
- `${env:PATH}`: 系统PATH环境变量
- `${workspaceFolder}/build/Windows/Debug/bin`: 可执行文件目录

##### 环境变量配置
| 变量名 | 作用 | 路径说明 |
|--------|------|----------|
| `PATH` | 运行时库搜索路径 | Qt bin目录 + 项目插件目录 + 系统PATH |
| `QT_PLUGIN_PATH` | Qt插件路径 | Qt安装目录下的plugins文件夹 |
| `QT_QPA_PLATFORM_PLUGIN_PATH` | 平台插件路径 | Qt平台相关插件路径 |

##### 调试器配置
- `type: "cppdbg"`: 使用C++调试器
- `MIMode: "gdb"`: 使用GDB作为调试后端
- `miDebuggerPath`: 指定GDB可执行文件路径

#### 4. 使用方法

##### 通过状态栏
1. 点击Cursor状态栏上的"运行"或"调试"按钮
2. 选择对应的配置

##### 通过命令面板
1. 按 `Ctrl+Shift+P`
2. 输入 "Debug: Start Debugging"
3. 选择配置

##### 通过快捷键
- `F5`: 开始调试（使用默认配置）
- `Ctrl+F5`: 运行（无调试）

##### 通过调试面板
1. 按 `Ctrl+Shift+D` 打开调试面板
2. 在配置下拉菜单中选择配置
3. 点击绿色播放按钮

---

## settings.json - 项目设置配置

### 文件作用
`settings.json` 是VSCode/Cursor的项目设置文件，配置了CMake Tools、C++智能感知、文件关联等设置。

### 配置项详细说明

#### 1. CMake配置
```json
{
    "cmake.buildDirectory": "${workspaceFolder}/build",           // 构建目录
    "cmake.generator": "Ninja",                                   // 构建生成器
    "cmake.configureArgs": [                                      // CMake配置参数
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64"
    ],
    "cmake.buildArgs": [                                          // 构建参数
        "-j4"
    ],
    "cmake.configureOnOpen": false,                               // 打开时自动配置
    "cmake.buildBeforeRun": true                                  // 运行前自动构建
}
```

##### CMake配置项说明
| 配置项 | 作用 | 说明 |
|--------|------|------|
| `buildDirectory` | 构建目录 | 指定CMake构建输出目录 |
| `generator` | 构建生成器 | 使用Ninja构建系统 |
| `configureArgs` | 配置参数 | 传递给cmake命令的参数 |
| `buildArgs` | 构建参数 | 传递给构建命令的参数 |
| `configureOnOpen` | 自动配置 | 是否在打开项目时自动运行cmake配置 |
| `buildBeforeRun` | 运行前构建 | 是否在运行程序前自动构建 |

#### 2. C++配置
```json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",  // 配置提供者
    "C_Cpp.default.compilerPath": "C:/Qt/Tools/mingw1310_64/bin/g++.exe",  // 编译器路径
    "C_Cpp.default.cppStandard": "c++17"                              // C++标准
}
```

##### C++配置项说明
| 配置项 | 作用 | 说明 |
|--------|------|------|
| `configurationProvider` | 配置提供者 | 使用CMake Tools提供配置 |
| `compilerPath` | 编译器路径 | 指定C++编译器可执行文件路径 |
| `cppStandard` | C++标准 | 设置C++语言标准版本 |

#### 3. 文件关联配置
```json
{
    "files.associations": {
        "*.h": "cpp",        // C++头文件
        "*.cpp": "cpp",      // C++源文件
        "*.ui": "xml",       // Qt UI文件
        "*.qrc": "xml"       // Qt资源文件
    }
}
```

##### 文件关联说明
| 扩展名 | 关联类型 | 作用 |
|--------|----------|------|
| `*.h` | cpp | 启用C++语法高亮和智能感知 |
| `*.cpp` | cpp | 启用C++语法高亮和智能感知 |
| `*.ui` | xml | 启用XML语法高亮（Qt Designer文件） |
| `*.qrc` | xml | 启用XML语法高亮（Qt资源文件） |

#### 4. 调试环境配置
```json
{
    "cmake.debugConfig": {
        "environment": [
            {
                "name": "PATH",
                "value": "C:\\Qt\\6.9.1\\mingw_64\\bin;${workspaceFolder}\\build\\Windows\\Debug\\pluginspackage;${workspaceFolder}\\build\\Windows\\Release\\pluginspackage;${env:PATH}"
            },
            {
                "name": "QT_PLUGIN_PATH",
                "value": "C:\\Qt\\6.9.1\\mingw_64\\plugins"
            },
            {
                "name": "QT_QPA_PLATFORM_PLUGIN_PATH",
                "value": "C:\\Qt\\6.9.1\\mingw_64\\plugins\\platforms"
            }
        ]
    }
}
```

##### 调试环境说明
这个配置为CMake Tools的调试功能设置环境变量，确保：
- 程序能找到Qt运行时库
- 程序能找到项目插件库
- Qt插件系统正常工作

---

## 使用场景和最佳实践

### 1. 日常开发流程
1. **打开项目**: Cursor自动加载配置
2. **编写代码**: 享受智能感知和语法高亮
3. **构建项目**: 使用CMake Tools或快捷键
4. **调试程序**: 按F5开始调试
5. **运行程序**: 按Ctrl+F5直接运行

### 2. 调试技巧
- **设置断点**: 在代码行号左侧点击
- **查看变量**: 在调试面板的"变量"区域
- **调用栈**: 在调试面板的"调用栈"区域
- **控制台**: 在调试面板的"调试控制台"区域

### 3. 配置自定义
- **添加新的调试配置**: 在`launch.json`的`configurations`数组中添加
- **修改构建参数**: 在`settings.json`中修改`cmake.buildArgs`
- **更改编译器**: 修改`C_Cpp.default.compilerPath`

### 4. 故障排除
- **配置不生效**: 重启Cursor
- **智能感知不工作**: 检查编译器路径是否正确
- **调试失败**: 检查GDB路径和程序路径
- **环境变量问题**: 检查`launch.json`中的环境变量设置

---

## 总结

通过正确配置这两个JSON文件，您的Automation项目将获得：
- ✅ 完整的调试支持
- ✅ 智能的代码感知
- ✅ 自动的构建管理
- ✅ 类似Qt Creator的开发体验
- ✅ 无需手动配置的运行时环境

这些配置文件是项目在VSCode/Cursor环境中正常工作的关键，理解它们的作用有助于您更好地使用开发环境。
