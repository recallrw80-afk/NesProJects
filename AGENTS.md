# NES 模拟器（C++）— AGENTS.md

学习型单文件树 C++20 NES 模拟器。无测试 / CI / README / opencode 配置。入口：`main.cpp`，CMake 目标名 **`Nes`**。

## 教学模式（先读这个）

用户在学 6502/NES，约定是 **示范 + 用户自己写**：

- 同类指令只示范 1–2 条并讲原理，**其余留给用户**。禁止一次性实现全部指令、`reset/step/nmi/irq`，或整块 CPU 流程。
- 禁止“照抄 A 换成 B”空模板；讲心智模型：取指-译码-执行、寻址如何落到 `addr_abs`、谁改标志位。
- 放慢节奏，确认理解再前进。用户已掌握：hex/字节/bit、`& 0xFF` / `(hi<<8)|lo`、PC 前移、寻址统一框架（IZX 先加后读 / IZY 先读后加、跨页加周期）。
- 已修好的逻辑 bug 不要“顺手重写”。当前任务是**补缺失函数体**，不是重构。

若链接报大量 `undefined reference to CPU::…`，**这是预期状态**，不要为了“让工程绿”而代写全部指令。

## 代码现状（以 `src/CPU.cpp` 为准）

| 项 | 事实 |
| --- | --- |
| 完成 | Cartridge/Mapper0、Bus 解码、PPU 寄存器读写、CPU 13 种寻址 + 读写/栈/标志辅助函数 |
| 指令体 | **仅 19 条已定义**：LDA/LDX/LDY/STA/STX/STY、TAX/TAY/TXA/TYA/TSX/TXS、SEC/CLC/SED/CLD/SEI/CLI/CLV |
| 未定义 | **AND 也没有空壳**；其余官方指令、`XXX`、`reset`/`step`/`nmi`/`irq` 均未写 |
| 占位 | `src/Mapper.cpp`、`src/DMA.cpp`、`src/APU.cpp` 为空 |
| Bus | PPU 已转发；**APU/手柄 0x4000–0x4017 仍 stub** |

256 项操作码表已填完——链接错误来自表引用了未实现的成员函数，**不要改表来“消错”**。

下一步顺序（教学推进，勿打乱）：当前用户在写 **AND** → 之后同类逻辑运算（ORA/EOR）→ 算术/移位 → 分支/跳转 → 栈/中断 → 最后 `reset`/`step`/`nmi`/`irq`。

## 构建（本机已验证）

**已有 `build_debug` 缓存绑定 CLion + MinGW Makefiles。** 换 generator（例如改用 `CMakePresets.json` 的 Ninja）必须换二进制目录，或先删该目录下的 `CMakeCache.txt`。

```powershell
# 与现有 build_debug 一致（CLion 自带工具链）
$clion = "C:\Users\user001\AppData\Local\Programs\CLion"
$cmake = "$clion\bin\cmake\win\x64\bin\cmake.exe"
$gpp   = "$clion\bin\mingw\bin\g++.exe"   # g++ 15.2.0
& $cmake -S . -B build_debug -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$gpp"
& $cmake --build build_debug
```

易错点：

- CLion 在 **`%LOCALAPPDATA%\Programs\CLion`**，不是 `Program Files\JetBrains`。
- 第二套工具链（WinGet MinGW-W64：cmake 4.4.1 / g++ 16.1.0 / ninja 1.13.2）在  
  `…\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\`。`CMakePresets.json` 要求 Ninja。
- **不传 `-DCMAKE_BUILD_TYPE` 时 CMake 默认 Release**；`main.cpp` 里 `cart.print_info()` 只在 `DEBUG` 宏下编译，Release 跑 ROM 看不到卡带信息。
- 源收集在**根** `CMakeLists.txt`：`GLOB_RECURSE src/*.cpp` + `main.cpp`。`src/CMakeLists.txt` 是空文件，别往里加 target。
- GNU 警告当错误：`-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion`。
- 产物：`<build>/bin/Nes.exe`。无测试命令；验证 = 编译/链接，链完后：
  `build_debug\bin\Nes.exe ".ines\Super Mario Bros.nes"`
- 示例 ROM：`.ines/Super Mario Bros.nes`（路径含空格，记得加引号）。

## 架构（文件名看不出的部分）

- 头文件 `include/basic|nes/`，实现 `src/`。类型别名 `include/basic/types.h`（`u8`/`u16`/…）。
- **Bus 是中枢**：0x0000–0x1FFF = 2KB RAM 镜像；0x2000–0x3FFF → PPU 寄存器；0x4000–0x4017 → APU/IO（未接）；0x4020+ → Cartridge/Mapper。
- **CPU 设计**：寻址模式函数与指令函数均返回 **`void`**。寻址模式只写 `addr_abs`/`addr_rel`（跨页时 `cycles++`）并推进 `pc`；指令函数经 `fetch()` / `addr_abs` 读写，用 **`set_flags`** 或直接赋 `flag_*`（不是 `set_flag`）。
- 构造函数用初始化列表（`Bus`/`CPU`/`PPU`）；成员 `snake_case`，类 `PascalCase`。
- `docs/L01–L39.md` 是课程笔记（**缺 L19**）。部分笔记里的 `D:/code/rust/nes_core/...` 是 **Rust 参考实现路径**，不在本仓库，不要去改那些路径。

## 逻辑坑（编译器拦不住，`-Werror` 也拦不住）

这些曾是真 bug，审查时重点看：

1. Bus：PPU 区间与 APU 区间必须分开解码。
2. PPU 偏移：`$2003=OAMADDR`，`$2005=PPUSCROLL`，`$2006=PPUADDR`，`$2007=PPUDATA`。
3. `REL()`：`addr_rel = read_mem(pc++)`，为负时 `addr_rel |= 0xFF00`。
4. `IND()`：低字节用 `p_lo`；指针低字节 0xFF 时高字节从 `$xx00` 回绕（6502 硬件 bug）。

写指令时：先确认寻址已算出正确操作数地址，再写操作本体；结果影响 Z/N；**TXS 不改标志**。

## 禁止

- 不要替用户写完剩余指令或 CPU 控制流。
- 不要未经用户要求改动已验证的寻址模式/操作码表。
- 不要假设存在测试框架、除 ROM 外的 fixture，或 `cpp/` codegraph 约定。
