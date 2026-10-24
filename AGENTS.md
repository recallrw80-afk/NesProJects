# NES 模拟器（C++）— AGENTS.md

学习型单文件树 C++20 NES 模拟器。无测试 / CI / README / opencode 配置。入口：`main.cpp`，CMake 目标名 **`Nes`**。

## 也是最重要的约定:教学模式（先读这个）

用户在学 6502/NES，约定是 **示范 + 用户自己写**：

- include/,src/,CMakeLists.txt,main.cpp,CMakePresets.json 下的所有文件禁止改动,只能用户改动
- 所有示范代码只能输出到对话中,禁止写入以第一条的目录中
- 同类指令只示范 1–2 条并讲原理，**其余留给用户**。禁止一次性实现全部指令、`reset/step/nmi/irq`，或整块 CPU 流程。
- 禁止“照抄 A 换成 B”空模板；讲心智模型：取指-译码-执行、寻址如何落到 `addr_abs`、谁改标志位。
- 放慢节奏，确认理解再前进。用户已掌握：hex/字节/bit、`& 0xFF` / `(hi<<8)|lo`、PC 前移、寻址统一框架（IZX 先加后读 / IZY 先读后加、跨页加周期）。
- 已修好的逻辑 bug 不要“顺手重写”。当前任务是**补缺失函数体**，不是重构。

若链接报大量 `undefined reference to CPU::…`，**这是预期状态**，不要为了“让工程绿”而代写全部指令。

## 代码现状（以 `src/CPU.cpp` 为准）

| 项 | 事实 |
| --- | --- |
| 完成 | Cartridge/Mapper0、Bus 解码、PPU 寄存器读写、CPU 13 种寻址 + 全部辅助函数（`read_mem`/`write_mem`/`fetch`/`push`/`pull`/`get_flags`/`set_flags`） |
| 指令体 | **31 条已定义**：LDA/LDX/LDY/STA/STX/STY、TAX/TAY/TXA/TYA/TSX/TXS、SEC/CLC/SED/CLD/SEI/CLI/CLV、AND/ORA/EOR、INX/INY/DEX/DEY、INC/DEC、ADC/SBC、CMP |
| 未定义 | 移位 ASL/LSR/ROL/ROR；分支 BCC/BCS/BEQ/BNE/BMI/BPL/BVC/BVS；跳转 JMP/JSR/RTS/RTI/BRK；栈 PHA/PHP/PLA/PLP；BIT/CPX/CPY/NOP；`XXX`；`reset`/`step`/`nmi`/`irq` |
| 占位 | `src/Mapper.cpp`、`src/DMA.cpp`、`src/APU.cpp` 为空 |
| Bus | PPU 已转发；**APU/手柄 0x4000–0x4017 仍 stub** |
| 已知未修 | `CMP()` 比较逻辑错（先截断成 `u8` 再比）；`ADC()`/`SBC()` 均未设 `flag_v`；十进制模式未处理 —— 留给用户自己改，别抢着重写 |

256 项操作码表已填完——链接错误来自表引用了未实现的成员函数，**不要改表来“消错”**。

下一步顺序（教学推进，勿打乱）：逻辑运算（AND/ORA/EOR）与 INC/DEC、ADC/SBC、CMP 已完成 → 当前段是**算术/移位**剩下的 ASL/LSR/ROL/ROR（可捎带 CPX/CPY/BIT/NOP）→ 分支/跳转 → 栈/中断 → 最后 `reset`/`step`/`nmi`/`irq`。

## 构建（两台机器，路径不同——先确认自己在哪台）

**正常运行环境是 user001 那台机（长期存在）**；EDY 这台只是**临时用机，随时可能不存在**。两台机的工具链位置和构建目录不一样，照抄另一台机的命令会找不到编译器。

共同前提：每台机的构建目录都已绑 **CLion 自带工具链 + MinGW Makefiles + Debug**。换 generator（例如改用 `CMakePresets.json` 的 Ninja）必须换二进制目录，或先删该目录下的 `CMakeCache.txt`。

### A. 临时用机（EDY，随时可能不存在）

- 定位：仅用于临时验证/上手。这台机的 CLion 安装和 `cmake-build-debug` 目录**都可能哪天就没了**，不要把它当唯一构建方式。
- CLion：**`C:\Program Files\JetBrains\CLion 2026.2.2`**（cmake 4.3.1 / g++ 15.2.0 / mingw32-make）。`cmake-build-debug/CMakeCache.txt` 里的 `CMAKE_CXX_COMPILER` / `CMAKE_MAKE_PROGRAM` 就是这套。
- 构建目录：**`cmake-build-debug`**（CLion 默认）。
- **`cmake` / `ninja` / `g++` / `mingw32-make` 全都不在 PATH 上**（`command -v` 找不到任何编译器）。所以 configure 时**必须显式传 `CMAKE_MAKE_PROGRAM`**，否则直接报错：
  `CMake Error: CMake was unable to find a build program corresponding to "MinGW Makefiles".`
- CLion 其实自带 ninja 1.13.2（`bin\ninja\win\x64\ninja.exe`），但它同样不在 PATH，`CMakePresets.json` 想用也得手传 `CMAKE_MAKE_PROGRAM`；默认不走这条路，直接用下面的 MinGW Makefiles 更省事。
- 这台机上没有 `%LOCALAPPDATA%\Programs\CLion`、也没有 `C:\Users\user001\...` 那份安装。

```powershell
# 临时用机（EDY）：CLion 2026.2.2 自带工具链
# 四个路径变量都必须绝对路径，PATH 上什么都没有
$clion  = "C:\Program Files\JetBrains\CLion 2026.2.2"
$cmake  = "$clion\bin\cmake\win\x64\bin\cmake.exe"
$gpp    = "$clion\bin\mingw\bin\g++.exe"          # g++ 15.2.0
$make   = "$clion\bin\mingw\bin\mingw32-make.exe"
& $cmake -S . -B cmake-build-debug -G "MinGW Makefiles" `
    -DCMAKE_BUILD_TYPE=Debug `
    -DCMAKE_CXX_COMPILER="$gpp" `
    -DCMAKE_MAKE_PROGRAM="$make" `
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
& $cmake --build cmake-build-debug -j 8
```

已搭好的状态（2026-10-24 实测）：configure 通过 → 8 个编译单元全部编过、**0 warning 0 error** → 链接阶段报 25 个 `undefined reference to CPU::ASL/CPX/BEQ/XXX/…`。**这是预期状态**（指令体还没写），环境本身没问题：同一套 g++ 能正常编出并运行独立 exe。

### B. 正常运行环境（user001 那台机）

- 定位：**长期存在的主环境**，正常开发就在这里跑。
- CLion：**`C:\Users\user001\AppData\Local\Programs\CLion`**，即 `%LOCALAPPDATA%\Programs\CLion`，**不是** `Program Files\JetBrains`。
- 构建目录：**`build_debug`**。
- 另有第二套工具链 **WinGet MinGW-W64**（cmake 4.4.1 / g++ 16.1.0 / ninja 1.13.2），位于  
  `…\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\`。这台机上 `CMakePresets.json` 的 Ninja 路线可用。

```powershell
# 正常运行环境（user001）：与现有 build_debug 一致（CLion 自带工具链）
$clion = "C:\Users\user001\AppData\Local\Programs\CLion"
$cmake = "$clion\bin\cmake\win\x64\bin\cmake.exe"
$gpp   = "$clion\bin\mingw\bin\g++.exe"   # g++ 15.2.0
& $cmake -S . -B build_debug -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$gpp"
& $cmake --build build_debug
```

若这台机也报 `unable to find a build program corresponding to "MinGW Makefiles"`，同样补一句 `-DCMAKE_MAKE_PROGRAM="$clion\bin\mingw\bin\mingw32-make.exe"` 即可。

### 两台机共通易错点

- **不传 `-DCMAKE_BUILD_TYPE` 时 CMake 默认 Release**；`main.cpp` 里 `cart.print_info()` 只在 `DEBUG` 宏下编译，Release 跑 ROM 看不到卡带信息。
- 源收集在**根** `CMakeLists.txt`：`GLOB_RECURSE src/*.cpp` + `main.cpp`。`src/CMakeLists.txt` 是空文件，别往里加 target。
- GNU 警告当错误：`-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion`。
- 产物：`<build>/bin/Nes.exe`。无测试命令；验证 = 编译/链接，链完后跑：
  - 正常运行环境（user001）：`build_debug\bin\Nes.exe ".ines\Super Mario Bros.nes"`
  - 临时用机（EDY）：`cmake-build-debug\bin\Nes.exe ".ines\Super Mario Bros.nes"`
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
