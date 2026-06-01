# PRD: MiniSTA — 轻量级静态时序分析器

> **版本**: v2.0  
> **日期**: 2026-06-01  
> **作者**: Alex（产品经理）  
> **状态**: Draft  
> **技术栈**: C++ / Tcl / CMake

---

## 1. 项目概述

### 1.1 一句话描述

一个用C++实现的轻量级静态时序分析工具，嵌入Tcl脚本引擎，提供类似Synopsys PrimeTime的命令行交互体验，帮助EE/CS学生深入理解STA原理并积累高质量简历项目。

### 1.2 项目定位

```
┌─────────────────────────────────────────────────────────────┐
│                    EDA工具生态定位                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   商用工具           开源工具           本项目                │
│   ─────────         ─────────         ─────────             │
│   PrimeTime    ←→   OpenSTA     ←→   MiniSTA               │
│   Tempus             OpenTimer         (学习级)              │
│   (Synopsys)         (Google)                                │
│                                                             │
│   功能完整           功能较完整         核心功能              │
│   License昂贵        开源但复杂         轻量易懂              │
│   企业级             研究级             学习级                │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 1.3 背景与动机

**用户痛点**：

| 痛点 | 现状 | MiniSTA解决方案 |
|------|------|-----------------|
| STA公式抽象 | 课堂上只讲公式，没有动手验证 | 命令行交互，输入参数即时计算 |
| 商用工具门槛高 | PrimeTime需要License，环境配置复杂 | 零依赖，CMake一键构建 |
| 开源工具太复杂 | OpenSTA代码量大，学习曲线陡 | 精简到核心功能，代码量<3000行 |
| 面试项目同质化 | 大部分简历项目是Python小工具 | C++系统级项目，展示工程能力 |

**项目价值**：

- **技术深度**：C++实现 + Tcl嵌入 + 编译系统 + 数据结构设计，远超普通Python项目
- **领域专业**：直接对标PrimeTime，面试时能讲"我写过一个简化版PT"
- **可扩展性**：架构清晰，后续可加SDF、Verilog网表解析等功能
- **开源价值**：代码精简、文档完整，有潜力获得社区关注

---

## 2. 目标用户画像

### 主要用户：EE/CS学生（求职导向）

| 维度 | 描述 |
|------|------|
| **背景** | 本科/研究生，数字IC设计或EDA方向 |
| **技术水平** | 有C++基础，了解数字电路和STA概念 |
| **使用场景** | 简历项目、面试准备、课程作业辅助 |
| **核心诉求** | 一个能写进简历、面试时能深入讲解的系统级项目 |
| **痛点** | 缺少能展示C++工程能力和EDA领域理解的项目 |

### 次要用户：EDA爱好者/初级工程师

| 维度 | 描述 |
|------|------|
| **背景** | 对EDA工具内部实现感兴趣的技术人员 |
| **核心诉求** | 理解STA工具的内部工作原理 |

---

## 3. 问题陈述

### 核心问题

> 学生在学习STA时，缺乏一个**轻量、可交互、可扩展**的工具来理解时序分析的内部逻辑；同时在求职时，缺少一个能展示C++工程能力和EDA领域理解的高质量项目。

### 用户故事（高层级）

```
作为一名EE/CS学生，
我希望有一个类似PrimeTime的命令行工具，能通过Tcl脚本定义时序路径、执行分析、生成报告，
以便我能深入理解STA工具的工作原理，并在面试中展示我的系统级开发能力。
```

### 成功标准

| 维度 | 目标 |
|------|------|
| **功能** | 支持核心Tcl命令集，完成Setup/Hold分析 |
| **准确性** | 计算结果与手动计算/教科书例题100%一致 |
| **用户体验** | 交互式Shell响应流畅，命令提示清晰 |
| **代码质量** | 核心模块测试覆盖率 > 80%，代码结构清晰 |
| **简历价值** | 能在面试中讲解10分钟以上，覆盖设计决策、技术选型、难点攻克 |

---

## 4. 核心功能需求

### 4.1 Tcl命令集设计

对标PrimeTime的命令风格，设计精简版命令集：

#### 时序对象定义命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `create_clock` | 定义时钟 | `create_clock -name clk -period 10` |
| `set_input_delay` | 设置输入延迟 | `set_input_delay -clock clk 2.0 [get_ports in1]` |
| `set_output_delay` | 设置输出延迟 | `set_output_delay -clock clk 1.5 [get_ports out1]` |
| `create_cell` | 创建单元（寄存器） | `create_cell -name reg1 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1` |
| `create_net` | 创建连线 | `create_net -name net1 -from reg1/Q -to reg2/D` |
| `set_comb_delay` | 设置组合逻辑延迟 | `set_comb_delay -net net1 -delay 3.0` |

#### 时序分析命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `report_timing` | 生成时序报告 | `report_timing -from reg1 -to reg2` |
| `report_timing -setup` | 仅报告Setup路径 | `report_timing -setup -from reg1 -to reg2` |
| `report_timing -hold` | 仅报告Hold路径 | `report_timing -hold -from reg1 -to reg2` |
| `check_timing` | 检查所有路径时序 | `check_timing` |
| `report_clock_skew` | 报告时钟偏移 | `report_clock_skew -clock clk` |

#### 查询与调试命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `get_cells` | 获取单元列表 | `get_cells reg*` |
| `get_nets` | 获取连线列表 | `get_nets *` |
| `get_clocks` | 获取时钟列表 | `get_clocks *` |
| `report_design` | 报告设计概要 | `report_design` |

#### 脚本控制命令

| 命令 | 功能 | 示例 |
|------|------|------|
| `source` | 执行Tcl脚本文件 | `source design.tcl` |
| `redirect` | 重定向输出到文件 | `redirect report.txt {report_timing}` |
| `help` | 显示命令帮助 | `help report_timing` |

### 4.2 核心计算引擎

#### Setup时序检查

```
Setup Slack = (T_clk + T_skew) - (T_cq + T_comb + T_setup)
```

- `T_clk`: 时钟周期（由`create_clock -period`定义）
- `T_cq`: Clock-to-Q延迟（由`create_cell -tcq`定义）
- `T_comb`: 组合逻辑延迟（由`set_comb_delay`定义）
- `T_setup`: 建立时间（由`create_cell -tsetup`定义）
- `T_skew`: 时钟偏移（由`create_clock -skew`或`report_clock_skew`计算）

#### Hold时序检查

```
Hold Slack = (T_cq + T_comb) - (T_hold - T_skew)
```

### 4.3 输出格式

#### 文本表格输出（report_timing）

```
============================================================================
                           Timing Report
============================================================================

Path Type: Setup
Startpoint: reg1 (DFF)
Endpoint:   reg2 (DFF)
Clock:      clk (period = 10.000ns)

--------------------------------------------------------------------------
  Pin/Port          Delay      Cumulative    Description
--------------------------------------------------------------------------
  reg1/CK           0.000      0.000         Clock input
  reg1/Q            0.500      0.500         Tcq delay
  net1              3.000      3.500         Combinational logic
  reg2/D            0.000      3.500         Data arrival
--------------------------------------------------------------------------
  Data Required Time:   10.000 + 0.000 - 0.200 = 9.800
  Data Arrival Time:    3.500
--------------------------------------------------------------------------
  Setup Slack:          6.300 (PASS)
============================================================================
```

#### ASCII时序图输出

```
Clock (clk):
    ┌───┐   ┌───┐   ┌───┐   ┌───┐
    │   │   │   │   │   │   │   │
────┘   └───┘   └───┘   └───┘   └──
    0   5   10  15  20  25  30  35

Data (reg1/Q → reg2/D):
              ┌───────────────────┐
              │   Tcq + Tcomb     │
              │   = 3.5ns         │
    ──────────┘                   └────────
              0.5                 4.0

Setup Check:
    Data Arrival:    3.500ns
    Data Required:   9.800ns
    Slack:           6.300ns ✓
```

### 4.4 功能优先级

| 优先级 | 功能模块 | 描述 | 复杂度 |
|--------|----------|------|--------|
| **P0** | Tcl引擎集成 | 嵌入libtcl，实现命令注册和执行 | ★★☆ |
| **P0** | 数据模型 | Cell、Net、Clock、Path等核心数据结构 | ★★☆ |
| **P0** | 核心计算 | Setup/Hold Slack计算引擎 | ★☆☆ |
| **P0** | 基础命令 | create_clock, create_cell, set_comb_delay, report_timing | ★★☆ |
| **P1** | 交互式Shell | tcl>提示符，历史命令，Tab补全 | ★★☆ |
| **P1** | 文本报告 | 格式化的report_timing输出 | ★☆☆ |
| **P1** | 脚本执行 | source命令，支持.tcl文件批量执行 | ★☆☆ |
| **P1** | 输入验证 | 错误提示，参数检查 | ★☆☆ |
| **P2** | ASCII时序图 | 终端中的时序波形可视化 | ★★☆ |
| **P2** | 多路径分析 | check_timing批量检查所有路径 | ★★☆ |
| **P2** | 命令补全 | Tab补全命令和对象名 | ★★☆ |
| **P2** | 结果重定向 | redirect命令输出到文件 | ★☆☆ |
| **P3** | 命令帮助系统 | help命令，显示用法和示例 | ★☆☆ |
| **P3** | 设计加载 | 简单的网表文件解析 | ★★★ |

---

## 5. 用户故事（详细）

### Epic 1: Tcl脚本引擎

#### US-1.1: 启动交互式Shell
```
作为一名学生，
我想启动minista后进入一个tcl>提示符，可以逐行输入命令，
以便我能像使用PrimeTime一样交互式地探索时序分析。
```

**验收标准**：
- [ ] 运行`./minista`后显示欢迎信息和`tcl>`提示符
- [ ] 输入Tcl命令后立即执行并显示结果
- [ ] 输入`exit`或`quit`退出程序
- [ ] 支持Tcl基本语法（变量赋值、puts、if、foreach等）
- [ ] 错误命令给出清晰的错误提示，不崩溃

#### US-1.2: 执行Tcl脚本文件
```
作为一名学生，
我想编写.tcl脚本文件，然后用source命令一次性执行，
以便我能自动化批量分析任务。
```

**验收标准**：
- [ ] `source design.tcl`能正确执行脚本文件中的所有命令
- [ ] 脚本中的错误会报告行号和错误信息
- [ ] 支持相对路径和绝对路径
- [ ] 脚本中的变量和过程定义在执行后仍可使用

#### US-1.3: 注册自定义命令
```
作为一名学生，
我想了解如何在Tcl中注册C++实现的自定义命令，
以便我能理解脚本引擎的内部工作原理。
```

**验收标准**：
- [ ] 每个STA命令都能正确注册到Tcl解释器
- [ ] 命令支持选项（-name, -period等）
- [ ] 命令返回值符合Tcl规范
- [ ] 错误参数给出usage提示

### Epic 2: 时序对象建模

#### US-2.1: 创建时钟
```
作为一名学生，
我想用create_clock命令定义时钟及其属性，
以便我能建立时序分析的参考基准。
```

**验收标准**：
- [ ] `create_clock -name clk -period 10`创建时钟对象
- [ ] 支持设置-skew属性
- [ ] `get_clocks *`列出所有时钟
- [ ] 重复创建同名时钟给出警告

#### US-2.2: 创建寄存器单元
```
作为一名学生，
我想用create_cell命令创建寄存器，并定义其时序参数，
以便我能建立时序路径的起点和终点。
```

**验收标准**：
- [ ] `create_cell -name reg1 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1`创建寄存器
- [ ] 支持DFF、LATCH等类型
- [ ] `get_cells reg*`支持通配符查询
- [ ] 缺少必要参数时给出错误提示

#### US-2.3: 创建连线和组合逻辑
```
作为一名学生，
我想用create_net和set_comb_delay定义寄存器之间的数据路径，
以便我能建立完整的时序路径。
```

**验收标准**：
- [ ] `create_net -name net1 -from reg1/Q -to reg2/D`创建连线
- [ ] `set_comb_delay -net net1 -delay 3.0`设置组合延迟
- [ ] 能验证from/to端口是否存在
- [ ] `get_nets *`列出所有连线

### Epic 3: 时序分析与报告

#### US-3.1: 执行Setup时序检查
```
作为一名学生，
我想用report_timing命令查看两点之间的Setup时序分析结果，
以便我能验证我的设计是否满足建立时间要求。
```

**验收标准**：
- [ ] `report_timing -from reg1 -to reg2`输出完整时序报告
- [ ] 报告包含Data Arrival Time、Data Required Time、Slack
- [ ] Slack ≥ 0显示PASS，Slack < 0显示FAIL
- [ ] 报告格式清晰，包含路径详情

#### US-3.2: 执行Hold时序检查
```
作为一名学生，
我想查看Hold时序分析结果，
以便我能验证设计是否满足保持时间要求。
```

**验收标准**：
- [ ] `report_timing -hold -from reg1 -to reg2`输出Hold报告
- [ ] Hold Slack计算正确
- [ ] Setup和Hold可以同时检查

#### US-3.3: 查看ASCII时序图
```
作为一名学生，
我想看到一个ASCII格式的时序图，直观展示信号传播，
以便我能更好地理解时序关系。
```

**验收标准**：
- [ ] `report_timing -diagram`显示ASCII时序图
- [ ] 图中清晰标注Tcq、Tcomb、Tsetup
- [ ] 标注Data Arrival和Data Required时间点
- [ ] 用符号区分PASS/FAIL

#### US-3.4: 批量检查所有路径
```
作为一名学生，
我想用check_timing一次性检查设计中所有路径，
以便我能快速发现时序违例。
```

**验收标准**：
- [ ] `check_timing`遍历所有寄存器对
- [ ] 输出汇总：总路径数、PASS数、FAIL数
- [ ] 列出所有违例路径及其Slack
- [ ] 支持`-setup_only`和`-hold_only`选项

---

## 6. 非功能性需求

| 维度 | 要求 |
|------|------|
| **性能** | 单路径计算 < 1ms，100条路径批量计算 < 100ms |
| **可移植性** | 支持Linux、macOS，Windows（WSL） |
| **可维护性** | 模块分离清晰，核心计算与Tcl接口解耦 |
| **可测试性** | 核心逻辑有单元测试，Tcl命令有集成测试 |
| **代码规范** | 遵循Google C++ Style Guide |
| **文档** | README完整，命令有help文档，关键代码有注释 |

---

## 7. 技术架构

### 7.1 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                      MiniSTA Architecture                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐      ┌──────────────────────────────┐    │
│  │   Tcl Shell  │      │      Tcl Script Engine       │    │
│  │  (交互模式)   │      │        (脚本模式)            │    │
│  └──────┬───────┘      └──────────────┬───────────────┘    │
│         │                             │                     │
│         └──────────┬──────────────────┘                     │
│                    │                                        │
│         ┌──────────▼──────────┐                            │
│         │   Command Dispatch  │                            │
│         │   (命令分发层)       │                            │
│         └──────────┬──────────┘                            │
│                    │                                        │
│  ┌─────────────────┼─────────────────┐                     │
│  │                 │                 │                     │
│  ▼                 ▼                 ▼                     │
│ ┌─────────┐  ┌──────────┐  ┌──────────────┐              │
│ │ Clock   │  │ Cell     │  │ Net          │              │
│ │ Commands│  │ Commands │  │ Commands     │              │
│ └────┬────┘  └────┬─────┘  └──────┬───────┘              │
│      │            │               │                       │
│      └────────────┼───────────────┘                       │
│                   │                                        │
│         ┌─────────▼─────────┐                             │
│         │   Data Model      │                             │
│         │   (数据模型层)     │                             │
│         └─────────┬─────────┘                             │
│                   │                                        │
│         ┌─────────▼─────────┐                             │
│         │  STA Calculator   │                             │
│         │  (时序计算引擎)    │                             │
│         └─────────┬─────────┘                             │
│                   │                                        │
│         ┌─────────▼─────────┐                             │
│         │  Report Generator │                             │
│         │  (报告生成器)      │                             │
│         └───────────────────┘                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 7.2 项目目录结构

```
minista/
├── CMakeLists.txt              # 顶层CMake配置
├── README.md                   # 项目说明
├── LICENSE
│
├── src/
│   ├── CMakeLists.txt
│   ├── main.cpp                # 程序入口
│   │
│   ├── tcl/                    # Tcl引擎层
│   │   ├── tcl_interp.h/cpp    # Tcl解释器封装
│   │   ├── tcl_command.h/cpp   # 命令基类
│   │   └── commands/           # 具体命令实现
│   │       ├── clock_cmds.h/cpp
│   │       ├── cell_cmds.h/cpp
│   │       ├── net_cmds.h/cpp
│   │       ├── report_cmds.h/cpp
│   │       └── utility_cmds.h/cpp
│   │
│   ├── model/                  # 数据模型层
│   │   ├── clock.h/cpp         # 时钟对象
│   │   ├── cell.h/cpp          # 单元（寄存器）对象
│   │   ├── net.h/cpp           # 连线对象
│   │   ├── port.h/cpp          # 端口对象
│   │   ├── design.h/cpp        # 设计容器
│   │   └── timing_path.h/cpp   # 时序路径
│   │
│   ├── engine/                 # 计算引擎层
│   │   ├── sta_engine.h/cpp    # STA计算核心
│   │   ├── slack_calculator.h/cpp  # Slack计算
│   │   └── path_finder.h/cpp   # 路径搜索
│   │
│   ├── report/                 # 报告生成层
│   │   ├── report_formatter.h/cpp  # 文本格式化
│   │   ├── timing_table.h/cpp      # 表格输出
│   │   └── ascii_diagram.h/cpp     # ASCII时序图
│   │
│   └── utils/                  # 工具层
│       ├── logger.h/cpp        # 日志
│       ├── string_utils.h/cpp  # 字符串工具
│       └── file_utils.h/cpp    # 文件工具
│
├── include/                    # 公共头文件
│   └── minista/
│       ├── minista.h           # 主头文件
│       └── types.h             # 类型定义
│
├── tests/                      # 测试
│   ├── CMakeLists.txt
│   ├── test_slack_calculator.cpp
│   ├── test_design.cpp
│   ├── test_tcl_commands.tcl   # Tcl集成测试
│   └── testdata/               # 测试数据
│       └── simple_design.tcl
│
├── examples/                   # 示例脚本
│   ├── basic_usage.tcl
│   ├── setup_violation.tcl
│   └── hold_violation.tcl
│
├── docs/                       # 文档
│   ├── PRD.md                  # 本文档
│   ├── ARCHITECTURE.md         # 架构说明
│   └── COMMANDS.md             # 命令参考
│
└── third_party/                # 第三方库
    └── tcl/                    # Tcl源码（或CMake FetchContent）
```

### 7.3 技术栈

| 组件 | 技术选择 | 理由 |
|------|----------|------|
| **语言** | C++17 | 现代特性，性能好，简历加分 |
| **构建系统** | CMake | 跨平台，工业标准 |
| **脚本引擎** | Tcl 8.6 | EDA行业标准，PrimeTime用的就是Tcl |
| **Tcl集成** | libtcl (系统安装 或 FetchContent) | 成熟稳定 |
| **测试框架** | Google Test | C++测试标准 |
| **代码规范** | Google C++ Style Guide | 业界认可 |
| **文档** | Markdown + Doxygen（可选） | 简洁易维护 |

### 7.4 关键设计决策

#### 为什么选Tcl而不是Python/Lua？

| 对比维度 | Tcl | Python | Lua |
|----------|-----|--------|-----|
| EDA行业标准 | ✅ PrimeTime/DC/VCS都用 | ❌ | ❌ |
| C++嵌入难度 | ★☆☆ 简单 | ★★★ 复杂 | ★★☆ 中等 |
| 简历价值 | ✅ 能讲"对标PT" | ❌ 太普通 | ❌ 无人问津 |
| 学习曲线 | ★★☆ | ★☆☆ | ★★☆ |

#### 数据模型设计

```cpp
// 核心数据结构示意
class Clock {
    std::string name;
    double period;      // ns
    double skew;        // ns
};

class Cell {
    std::string name;
    CellType type;      // DFF, LATCH
    double tcq;         // Clock-to-Q delay (ns)
    double tsetup;      // Setup time (ns)
    double thold;       // Hold time (ns)
};

class Net {
    std::string name;
    Cell* from_cell;
    std::string from_pin;
    Cell* to_cell;
    std::string to_pin;
    double comb_delay;  // Combinational delay (ns)
};

class TimingPath {
    Cell* startpoint;
    Cell* endpoint;
    Clock* clock;
    double data_arrival;
    double data_required;
    double slack;
    PathType type;      // SETUP, HOLD
};
```

---

## 8. 优先级排序（MoSCoW）

### Must Have（必须有）
- Tcl解释器嵌入和命令注册
- 核心数据模型（Clock, Cell, Net）
- Setup/Hold Slack计算引擎
- create_clock, create_cell, create_net, set_comb_delay命令
- report_timing命令（文本输出）
- 交互式Shell（tcl>提示符）
- source命令（脚本执行）
- 基本错误处理和输入验证

### Should Have（应该有）
- report_timing -diagram（ASCII时序图）
- check_timing（批量检查）
- get_cells, get_nets, get_clocks查询命令
- redirect命令（输出重定向）
- 单位支持（ns/ps切换）
- 格式化的时序报告表格

### Could Have（可以有）
- Tab命令补全
- help命令系统
- 命令历史（上下箭头）
- report_design概要报告
- 简单的网表文件解析

### Won't Have（本期不做）
- SDF反标
- Verilog网表解析
- 多时钟域分析
- 时序路径搜索（最短/最长路径）
- GUI界面

---

## 9. 成功指标

| 指标 | 目标值 | 衡量方式 |
|------|--------|----------|
| **功能完整性** | P0功能100%，P1功能80% | 所有Must Have可用，大部分Should Have可用 |
| **计算准确性** | 100% | 与手动计算、教科书例题一致 |
| **代码质量** | 核心模块测试覆盖率 > 80% | Google Test报告 |
| **用户体验** | 交互Shell响应 < 10ms | 主观体验 |
| **简历价值** | 面试能讲10分钟 | 自我评估，覆盖：架构设计、Tcl集成、难点攻克 |
| **开源价值** | README完整，有示例脚本 | 可直接clone运行 |

---

## 10. 项目里程碑（3-4周）

### Week 1: 基础框架 + 核心计算

**目标**：能编译运行，能执行基本计算

| 天数 | 任务 | 交付物 |
|------|------|--------|
| Day 1 | CMake项目搭建，Tcl库集成 | 能编译，能启动tcl>提示符 |
| Day 2 | 数据模型实现（Clock, Cell, Net） | 数据结构 + 单元测试 |
| Day 3 | 核心计算引擎（Slack Calculator） | 计算逻辑 + 单元测试 |
| Day 4 | Tcl命令注册框架 + create_clock/cell/net | 能通过Tcl命令创建对象 |
| Day 5 | report_timing基础版本 | 能输出文本格式时序报告 |

**里程碑验收**：
```tcl
tcl> create_clock -name clk -period 10
tcl> create_cell -name reg1 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1
tcl> create_cell -name reg2 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1
tcl> create_net -name net1 -from reg1/Q -to reg2/D
tcl> set_comb_delay -net net1 -delay 3.0
tcl> report_timing -from reg1 -to reg2
Setup Slack: 6.300 (PASS)
```

### Week 2: 完善交互 + 报告美化

**目标**：交互体验流畅，报告格式专业

| 天数 | 任务 | 交付物 |
|------|------|--------|
| Day 6 | Shell交互完善（历史、错误处理） | 友好的交互体验 |
| Day 7 | source命令 + 脚本执行 | 能运行.tcl脚本文件 |
| Day 8 | report_timing美化（格式化表格） | 专业的时序报告 |
| Day 9 | get_cells/nets/clocks查询命令 | 对象查询能力 |
| Day 10 | 输入验证 + 错误提示完善 | 健壮的错误处理 |

### Week 3: 高级功能 + 可视化

**目标**：功能完整，有可视化能力

| 天数 | 任务 | 交付物 |
|------|------|--------|
| Day 11 | ASCII时序图（ascii_diagram） | 可视化时序图 |
| Day 12 | check_timing批量检查 | 多路径分析 |
| Day 13 | Hold时序检查 | 完整的Setup/Hold分析 |
| Day 14 | redirect命令 + 输出重定向 | 结果导出能力 |
| Day 15 | 示例脚本 + 测试用例 | examples/目录 |

### Week 4: 打磨 + 文档（可选）

**目标**：可发布，简历可用

| 天数 | 任务 | 交付物 |
|------|------|--------|
| Day 16 | 命令帮助系统（help命令） | 用户文档 |
| Day 17 | README编写 + 架构文档 | 项目文档 |
| Day 18 | 代码重构 + 注释完善 | 可读性提升 |
| Day 19 | 性能优化 + 边界测试 | 质量保证 |
| Day 20 | 录制演示 + 准备面试话术 | 简历素材 |

---

## 11. 风险与依赖

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| Tcl库集成困难 | 阻塞开发 | 低 | Tcl文档完善，可用FetchContent自动下载 |
| 命令解析复杂 | 超出时间预算 | 中 | 先支持简单选项，后期再扩展 |
| 计算公式理解偏差 | 结果错误 | 低 | 参考教科书，用单元测试验证 |
| 项目范围蔓延 | 延期 | 中 | 严格按MoSCoW优先级，P3功能可砍 |

### 外部依赖

| 依赖 | 版本 | 获取方式 |
|------|------|----------|
| Tcl | 8.6+ | 系统包管理 或 CMake FetchContent |
| CMake | 3.14+ | 系统包管理 |
| C++编译器 | GCC 9+ / Clang 10+ | 系统包管理 |
| Google Test | 1.10+ | CMake FetchContent |

---

## 12. 开放问题

| 问题 | 选项 | 建议 |
|------|------|------|
| Tcl库获取方式 | 系统安装 vs FetchContent | 建议FetchContent，减少用户配置 |
| 是否支持通配符 | get_cells reg* | 建议支持，PT风格 |
| 时钟Skew默认值 | 0 vs 必须指定 | 建议默认0，可选指定 |
| 单位系统 | 仅ns vs ns/ps可切换 | 建议先做ns，后期加ps |

---

## 附录A: Tcl命令速查表

```tcl
# ====================
# 对象创建
# ====================
create_clock -name <name> -period <ns> [-skew <ns>]
create_cell  -name <name> -type <DFF|LATCH> -tcq <ns> -tsetup <ns> -thold <ns>
create_net   -name <name> -from <cell>/<pin> -to <cell>/<pin>
set_comb_delay -net <name> -delay <ns>

# ====================
# 对象查询
# ====================
get_clocks [<pattern>]
get_cells  [<pattern>]
get_nets   [<pattern>]

# ====================
# 时序分析
# ====================
report_timing -from <cell> -to <cell> [-setup|-hold] [-diagram]
check_timing  [-setup_only|-hold_only]
report_clock_skew [-clock <name>]
report_design

# ====================
# 脚本控制
# ====================
source <filename.tcl>
redirect <filename> {<command>}
help [<command>]
exit
```

---

## 附录B: 示例Tcl脚本

```tcl
# examples/basic_usage.tcl
# 基本使用示例

# 定义时钟
create_clock -name main_clk -period 10.0 -skew 0.1

# 定义寄存器
create_cell -name u_reg1 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1
create_cell -name u_reg2 -type DFF -tcq 0.5 -tsetup 0.2 -thold 0.1

# 定义数据路径
create_net -name data_path -from u_reg1/Q -to u_reg2/D
set_comb_delay -net data_path -delay 3.0

# 执行时序分析
puts "=== Timing Analysis ==="
report_timing -from u_reg1 -to u_reg2
report_timing -from u_reg1 -to u_reg2 -hold
report_timing -from u_reg1 -to u_reg2 -diagram

# 批量检查
puts "=== Check All Paths ==="
check_timing
```

---

## 附录C: 面试话术参考

**开场**：
> "我做了一个MiniSTA项目，用C++实现的轻量级静态时序分析器，嵌入了Tcl脚本引擎，交互方式对标PrimeTime。"

**技术深度**：
> "核心难点有几个：一是Tcl引擎的嵌入，需要理解Tcl的对象系统和命令注册机制；二是时序路径的数据建模，要设计出既简洁又可扩展的数据结构；三是报告格式化，要在终端里做出PT风格的表格输出。"

**设计决策**：
> "为什么选Tcl而不是Python？因为Tcl是EDA行业的标准脚本语言，PrimeTime、Design Compiler都用Tcl。这样做一方面学习了行业标准，另一方面面试时能直接对标PT，展示我对工具链的理解。"

**收获**：
> "通过这个项目，我不仅理解了STA的底层计算逻辑，还学会了如何设计一个带脚本引擎的命令行工具，包括命令分发、对象管理、错误处理这些系统级的设计模式。"

---

*文档结束*
