# MiniSTA 代码导读

> 帮你快速理解项目结构和核心代码逻辑。

---

## 1. 项目全景

```
sta-timing-calculator/
├── CMakeLists.txt              # 构建系统入口
├── build.sh                    # 编译脚本
├── bin/                        # 编译产物
│   ├── minista                 # C++主程序（Tcl Shell）
│   ├── minista.sh              # 启动脚本（支持--python/--tcl）
│   ├── minista_tests           # 单元测试
│   └── python/
│       └── minista.so          # Python绑定库
│
├── src/                        # 源代码
│   ├── main.cpp                # 程序入口 + Tcl Shell
│   └── core/
│       ├── sta_engine.h        # STA引擎头文件
│       └── sta_engine.cpp      # STA引擎实现
│
├── python/
│   ├── bindings.cpp            # pybind11绑定代码
│   └── CMakeLists.txt
│
├── include/minista/
│   ├── minista.h               # 版本信息、Banner
│   └── types.h                 # 枚举类型定义
│
├── tests/
│   └── test_main.cpp           # Google Test单元测试
│
├── examples/
│   └── basic_usage.tcl         # Tcl示例脚本
│
└── docs/
    ├── PRD.md                  # 产品需求文档
    └── CODE_GUIDE.md           # 本文档
```

---

## 2. 核心模块详解

### 2.1 数据模型 (`src/core/sta_engine.h`)

这是整个项目的核心，定义了时序分析所需的数据结构：

```cpp
// 时钟定义
struct Clock {
    std::string name;      // 时钟名，如 "clk"
    double period = 0.0;   // 时钟周期，单位 ns
    double skew = 0.0;     // 时钟偏移，单位 ns
};

// 单元（寄存器）定义
struct Cell {
    std::string name;      // 单元名，如 "reg1"
    std::string type;      // 类型："DFF" 或 "LATCH"
    double tcq = 0.0;      // Clock-to-Q 延迟 (ns)
    double tsetup = 0.0;   // 建立时间 (ns)
    double thold = 0.0;    // 保持时间 (ns)
};

// 连线定义
struct Net {
    std::string name;       // 连线名，如 "net1"
    std::string from_cell;  // 起始单元
    std::string from_pin;   // 起始引脚，如 "Q"
    std::string to_cell;    // 目标单元
    std::string to_pin;     // 目标引脚，如 "D"
    double delay = 0.0;     // 组合逻辑延迟 (ns)
};

// 时序分析结果
struct TimingResult {
    double data_arrival;    // 数据到达时间
    double data_required;   // 数据要求时间
    double slack;           = 0.0;  // 余量
    bool pass = false;      // 是否通过
    std::string path_type;  // "setup" 或 "hold"
};
```

**设计思路**：
- 用 `unordered_map` 存储所有对象，支持 O(1) 查找
- 每个对象都有 `name` 作为唯一标识
- 所有时间单位统一为 ns

---

### 2.2 STA引擎 (`src/core/sta_engine.cpp`)

核心类 `StaEngine` 提供以下功能：

```cpp
class StaEngine {
public:
    // === 对象创建 ===
    bool create_clock(name, period, skew);
    bool create_cell(name, type, tcq, tsetup, thold);
    bool create_net(name, from_cell, from_pin, to_cell, to_pin, delay);

    // === 对象查询 ===
    std::optional<Clock> get_clock(name);
    std::optional<Cell> get_cell(name);
    std::optional<Net> get_net(name);
    std::vector<std::string> get_all_clocks();
    std::vector<std::string> get_all_cells();
    std::vector<std::string> get_all_nets();

    // === 时序分析 ===
    TimingResult check_setup(from_cell, to_cell);
    TimingResult check_hold(from_cell, to_cell);
    std::string report_timing(from_cell, to_cell);

    // === 清空 ===
    void clear();

private:
    std::unordered_map<std::string, Clock> clocks_;
    std::unordered_map<std::string, Cell> cells_;
    std::unordered_map<std::string, Net> nets_;

    // 辅助函数：查找连接两个单元的连线
    std::optional<Net> find_net(from_cell, to_cell);
};
```

---

### 2.3 时序计算公式

**Setup检查**：

```
Setup Slack = (T_clk + T_skew) - (T_cq + T_comb + T_setup)

其中：
  T_clk   = 时钟周期
  T_skew  = 时钟偏移
  T_cq    = Clock-to-Q 延迟
  T_comb  = 组合逻辑延迟
  T_setup = 建立时间

判断：
  Slack >= 0 → PASS
  Slack < 0  → FAIL
```

**Hold检查**：

```
Hold Slack = (T_cq + T_comb) - (T_hold - T_skew)

其中：
  T_hold = 保持时间

判断：
  Slack >= 0 → PASS
  Slack < 0  → FAIL
```

**代码实现** (`check_setup` 函数)：

```cpp
TimingResult StaEngine::check_setup(const std::string& from_cell_name,
                                    const std::string& to_cell_name) const {
    TimingResult result;
    result.path_type = "setup";

    // 1. 获取源单元、目标单元、连线
    auto from_cell = get_cell(from_cell_name);
    auto to_cell = get_cell(to_cell_name);
    auto net = find_net(from_cell_name, to_cell_name);

    // 2. 检查对象是否存在
    if (!from_cell || !to_cell || !net) {
        result.pass = false;
        return result;
    }

    // 3. 获取时钟参数（简化：使用第一个时钟）
    double clock_period = 10.0;
    double clock_skew = 0.0;
    if (!clocks_.empty()) {
        const auto& clk = clocks_.begin()->second;
        clock_period = clk.period;
        clock_skew = clk.skew;
    }

    // 4. 计算 Slack
    result.data_arrival = from_cell->tcq + net->delay;
    result.data_required = clock_period + clock_skew - to_cell->tsetup;
    result.slack = result.data_required - result.data_arrival;
    result.pass = (result.slack >= 0);

    return result;
}
```

---

### 2.4 Tcl Shell (`src/main.cpp`)

**程序入口**：

```cpp
int main(int argc, char* argv[]) {
    // 1. 创建 Tcl 解释器
    Tcl_Interp* interp = CreateInterpreter();

    // 2. 注册自定义命令
    RegisterCommands(interp);

    // 3. 运行交互式 REPL
    RunRepl(interp);

    // 4. 清理
    Tcl_DeleteInterp(interp);
    return EXIT_SUCCESS;
}
```

**命令注册**：

```cpp
void RegisterCommands(Tcl_Interp* interp) {
    // 注册 hello 命令
    Tcl_CreateObjCommand(interp, "hello", HelloCmd, nullptr, nullptr);

    // 注册 minista_version 命令
    Tcl_CreateObjCommand(interp, "minista_version", MinistaVersionCmd,
                         nullptr, nullptr);
}
```

**自定义命令实现**（以 `hello` 为例）：

```cpp
int HelloCmd(ClientData client_data, Tcl_Interp* interp,
             int objc, Tcl_Obj* const objv[]) {
    // objc = 参数个数，objv = 参数数组

    if (objc > 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "?name?");
        return TCL_ERROR;
    }

    std::string name = "World";
    if (objc == 2) {
        name = Tcl_GetString(objv[1]);
    }

    std::string result = "Hello, " + name + "!";
    Tcl_SetObjResult(interp, Tcl_NewStringObj(result.c_str(), -1));
    return TCL_OK;
}
```

**REPL 循环**：

```cpp
void RunRepl(Tcl_Interp* interp) {
    char line[4096];

    while (true) {
        printf("tcl> ");
        fflush(stdout);

        // 读取用户输入
        if (fgets(line, sizeof(line), stdin) == nullptr) {
            break;  // EOF
        }

        // 检查退出命令
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        }

        // 执行 Tcl 命令
        int result = Tcl_Eval(interp, line);

        if (result == TCL_OK) {
            // 打印结果
            printf("%s\n", Tcl_GetStringResult(interp));
        } else {
            // 打印错误
            printf("Error: %s\n", Tcl_GetStringResult(interp));
        }
    }
}
```

---

### 2.5 Python绑定 (`python/bindings.cpp`)

使用 pybind11 将 C++ 类暴露给 Python：

```cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(minista, m) {
    m.doc() = "MiniSTA Python binding";

    // 绑定 TimingResult 结构体
    py::class_<TimingResult>(m, "TimingResult")
        .def_readonly("data_arrival", &TimingResult::data_arrival)
        .def_readonly("data_required", &TimingResult::data_required)
        .def_readonly("slack", &TimingResult::slack)
        .def_readonly("is_pass", &TimingResult::pass);  // 'pass' 是 Python 保留字

    // 绑定 StaEngine 类
    py::class_<StaEngine>(m, "StaEngine")
        .def(py::init<>())  // 构造函数
        .def("create_clock", &StaEngine::create_clock,
             py::arg("name"), py::arg("period"), py::arg("skew") = 0.0)
        .def("create_cell", &StaEngine::create_cell,
             py::arg("name"), py::arg("type") = "DFF",
             py::arg("tcq") = 0.0, py::arg("tsetup") = 0.0, py::arg("thold") = 0.0)
        .def("check_setup", &StaEngine::check_setup)
        .def("check_hold", &StaEngine::check_hold)
        .def("report_timing", &StaEngine::report_timing);
}
```

**关键点**：
- `py::class_<T>` 绑定 C++ 类
- `def_readonly` 绑定只读属性
- `def` 绑定成员函数
- `py::arg` 定义参数名和默认值
- `pass` 是 Python 保留字，所以用 `is_pass`

---

### 2.6 类型定义 (`include/minista/types.h`)

定义了项目使用的枚举类型：

```cpp
// 单元类型
enum class CellType : uint8_t {
    kDff = 0,
    kLatch = 1,
    kUnknown = 255,
};

// 路径类型
enum class PathType : uint8_t {
    kSetup = 0,
    kHold = 1,
};

// 时序结果
enum class TimingResult : uint8_t {
    kPass = 0,
    kFail = 1,
};

// 辅助函数：枚举转字符串
inline const char* CellTypeToString(CellType type) {
    switch (type) {
        case CellType::kDff:    return "DFF";
        case CellType::kLatch:  return "LATCH";
        default:                return "UNKNOWN";
    }
}
```

---

### 2.7 版本信息 (`include/minista/minista.h`)

```cpp
namespace minista {

struct Version {
    static constexpr int kMajor = 0;
    static constexpr int kMinor = 1;
    static constexpr int kPatch = 0;

    static const char* String() {
        return "0.1.0";
    }
};

constexpr const char* kProjectName = "MiniSTA";
constexpr const char* kProjectDescription = "Lightweight Static Timing Analyzer";

// ASCII Art Banner
constexpr const char* kProjectBanner = R"(
 ███╗   ███╗ ██╗ ███╗   ██╗ ...
 ...
)";

}  // namespace minista
```

---

## 3. 数据流

### 3.1 Tcl 命令执行流程

```
用户输入: "create_clock -name clk -period 10"
    │
    ▼
┌─────────────────┐
│   RunRepl()     │  读取输入
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Tcl_Eval()    │  Tcl 解释器执行
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  CreateClockCmd │  自定义命令处理
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ StaEngine::     │  调用核心引擎
│ create_clock()  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  clocks_["clk"] │  存储到 HashMap
│  = {10.0, 0.0}  │
└─────────────────┘
```

### 3.2 Python 调用流程

```python
# Python 代码
engine = minista.StaEngine()
engine.create_clock("clk", 10.0)
result = engine.check_setup("reg1", "reg2")
```

```
Python 调用
    │
    ▼
┌─────────────────┐
│  pybind11 绑定层 │  参数转换 Python → C++
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  StaEngine::    │  C++ 核心逻辑
│  check_setup()  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  TimingResult   │  结果转换 C++ → Python
│  → Python dict  │
└─────────────────┘
```

---

## 4. 构建系统

### 4.1 CMake 结构

```
CMakeLists.txt (顶层)
├── find_package(Tcl)          # 查找系统 Tcl
├── FetchContent(googletest)   # 下载 Google Test
├── FetchContent(pybind11)    # 下载 pybind11
│
├── add_subdirectory(src)      # 构建主程序 + 核心库
│   ├── minista_core (STATIC)  # 核心库
│   └── minista (EXECUTABLE)   # 主程序
│
├── add_subdirectory(tests)    # 构建测试
│   └── minista_tests
│
└── add_subdirectory(python)   # 构建 Python 绑定
    └── minista_py (MODULE)
```

### 4.2 依赖关系

```
minista (主程序)
    ├── Tcl::tcl          # Tcl 库
    └── minista_core      # 核心库

minista_py (Python 绑定)
    ├── pybind11          # pybind11
    └── minista_core      # 核心库

minista_tests (测试)
    ├── GTest::gtest      # Google Test
    ├── Tcl::tcl          # Tcl 库
    └── minista_core      # 核心库
```

### 4.3 关键 CMake 变量

| 变量 | 作用 |
|------|------|
| `CMAKE_CXX_STANDARD` | C++ 标准版本 (17) |
| `CMAKE_RUNTIME_OUTPUT_DIRECTORY` | 可执行文件输出目录 |
| `MINISTA_BUILD_TESTS` | 是否构建测试 |
| `MINISTA_BUILD_PYTHON` | 是否构建 Python 绑定 |
| `POSITION_INDEPENDENT_CODE` | 启用 -fPIC（Python 需要）|

---

## 5. 单元测试

### 5.1 测试结构 (`tests/test_main.cpp`)

```cpp
#include <gtest/gtest.h>
#include "minista/minista.h"

// 版本测试
TEST(VersionTest, StringFormatIsCorrect) {
    EXPECT_STREQ(Version::String(), "0.1.0");
}

// 类型测试
TEST(CellTypeTest, DffToString) {
    EXPECT_STREQ(CellTypeToString(CellType::kDff), "DFF");
}

// Tcl 集成测试
class TclInterpreterTest : public ::testing::Test {
protected:
    void SetUp() override {
        interp_ = Tcl_CreateInterp();
    }
    void TearDown() override {
        Tcl_DeleteInterp(interp_);
    }
    Tcl_Interp* interp_;
};

TEST_F(TclInterpreterTest, BasicTclEvaluation) {
    int result = Tcl_Eval(interp_, "expr {2 + 3}");
    EXPECT_EQ(result, TCL_OK);
    EXPECT_STREQ(Tcl_GetStringResult(interp_), "5");
}
```

### 5.2 运行测试

```bash
cd build && ctest --output-on-failure
```

---

## 6. 如何扩展

### 6.1 添加新的 Tcl 彮令

**步骤 1**：在 `src/main.cpp` 中实现命令函数

```cpp
int CreateClockCmd(ClientData /*client_data*/, Tcl_Interp* interp,
                   int objc, Tcl_Obj* const objv[]) {
    // 解析参数
    // 调用 StaEngine
    // 返回结果
    return TCL_OK;
}
```

**步骤 2**：在 `RegisterCommands` 中注册

```cpp
void RegisterCommands(Tcl_Interp* interp) {
    Tcl_CreateObjCommand(interp, "create_clock", CreateClockCmd,
                         nullptr, nullptr);
}
```

### 6.2 添加新的 Python 绑定

**步骤 1**：在 `python/bindings.cpp` 中添加绑定

```cpp
py::class_<StaEngine>(m, "StaEngine")
    .def("new_method", &StaEngine::new_method,
         py::arg("param1"), py::arg("param2") = 0.0,
         R"(
         方法说明文档
         )");
```

**步骤 2**：重新编译

```bash
./build.sh python
```

### 6.3 添加新的核心功能

**步骤 1**：在 `src/core/sta_engine.h` 中声明

```cpp
class StaEngine {
public:
    // 新功能
    TimingResult new_analysis(param1, param2);
};
```

**步骤 2**：在 `src/core/sta_engine.cpp` 中实现

```cpp
TimingResult StaEngine::new_analysis(param1, param2) {
    // 实现逻辑
}
```

**步骤 3**：暴露给 Tcl 和 Python

---

## 7. 关键代码位置速查

| 功能 | 文件 | 行号 |
|------|------|------|
| 程序入口 | `src/main.cpp` | `main()` |
| Tcl REPL | `src/main.cpp` | `RunRepl()` |
| 命令注册 | `src/main.cpp` | `RegisterCommands()` |
| 数据结构 | `src/core/sta_engine.h` | `struct Clock/Cell/Net` |
| Setup 计算 | `src/core/sta_engine.cpp` | `check_setup()` |
| Hold 计算 | `src/core/sta_engine.cpp` | `check_hold()` |
| 报告生成 | `src/core/sta_engine.cpp` | `report_timing()` |
| Python 绑定 | `python/bindings.cpp` | `PYBIND11_MODULE` |
| 类型定义 | `include/minista/types.h` | `enum class` |
| 版本信息 | `include/minista/minista.h` | `Version` |

---

## 8. 调试技巧

### 8.1 查看编译产物

```bash
ls -la bin/
ls -la bin/python/
```

### 8.2 运行单个测试

```bash
cd build
./bin/minista_tests --gtest_filter="VersionTest.*"
```

### 8.3 查看 CMake 配置

```bash
cd build
cmake -LH ..
```

### 8.4 Python 调试

```python
import sys
sys.path.insert(0, 'bin/python')
import minista

# 查看模块内容
print(dir(minista))

# 查看类方法
print(dir(minista.StaEngine))
```

---

## 9. 下一步学习建议

1. **先读 `src/core/sta_engine.h`** — 理解数据结构
2. **再读 `src/core/sta_engine.cpp`** — 理解计算逻辑
3. **然后读 `src/main.cpp`** — 理解 Tcl 集成
4. **最后读 `python/bindings.cpp`** — 理解 Python 绑定
5. **运行测试** — `cd build && ctest`
6. **尝试修改** — 添加一个新命令或新功能

---

*文档完成。祝你学习愉快！*
