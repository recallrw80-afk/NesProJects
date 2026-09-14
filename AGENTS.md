# NES 模拟器（C++）项目记忆

## 项目状态

### 已完成组件

| 文件                      | 状态    | 说明                                                                                                       |
| ------------------------- | ------- | ---------------------------------------------------------------------------------------------------------- |
| `include/basic/types.h`   | ✅ 完成 | u8/u16/u32/u64/i8/i16/i32/i64/f32/f64 类型别名                                                             |
| `include/nes/Mapper.h`    | ✅ 完成 | Mapper 抽象基类 (cpu_map_read/write, ppu_map_read/write)                                                   |
| `include/nes/Mapper0.h`   | ✅ 完成 | Mapper0 (NROM) 实现，无 bank switching                                                                     |
| `include/nes/Cartridge.h` | ✅ 完成 | Cartridge 声明：PRG/CHR ROM、SRAM、Mapper、iNES 头部字段                                                   |
| `src/Cartridge.cpp`       | ✅ 完成 | iNES 文件加载、头部解析、Mapper 创建、CPU/PPU 读写转发                                                     |
| `include/nes/Bus.h`       | ✅ 完成 | Bus 声明：cpu_ram (2KB)、Cartridge/CPU/PPU 指针、connect_cpu/connect_ppu                                   |
| `src/Bus.cpp`             | ✅ 完成 | 64KB 地址空间解码，CPU RAM 读写，已连接 PPU（寄存器读写已转发）；APU 段仍 return 0                              |
| `include/nes/PPU.h`       | ✅ 完成 | PPU 声明：VRAM (2KB)、调色板 (32B)、8 个寄存器、写锁存、读缓冲                                             |
| `src/PPU.cpp`             | ✅ 完成 | PPU 寄存器读写完整逻辑（PPUCTRL/PPUMASK/PPUSTATUS/PPUSCROLL/PPUADDR/PPUDATA），VRAM/调色板镜像，读缓冲延迟；寄存器偏移常量命名($2003/2005/2006/2007)已修正 |
| `include/nes/CPU.h`       | 🚧 进行中 | 寄存器/标志位/寻址模式/指令函数声明、Instruction 结构体与 256 项操作码表声明（已补 TYA 声明）             |
| `src/CPU.cpp`             | 🚧 进行中 | 256 项操作码表（已验证：151 条官方指令全部正确，其余 XXX）、辅助函数、**寻址模式 13/13 全部已写**（IMP/IMM/ZP0/ZPX/ZPY/ABX/ABY/ABS/IZX/IZY/ACC/REL/IND）、REL/IND 的 3 个逻辑 bug 已修复（commit d8ed3c1）；指令函数体只实现 19 条（加载/存储/传送/标志位）→ 编译全过、链接失败（缺 37 条 + XXX）属正常 |
| `src/Mapper.cpp`          | 📄 空   | 占位                                                                                                       |
| `src/DMA.cpp`             | 📄 空   | 占位                                                                                                       |
| `src/APU.cpp`             | 📄 空   | 占位                                                                                                       |
| `main.cpp`                | ✅ 完成 | 加载 ROM、打印 Cartridge 信息（DEBUG 模式）                                                                |
| `CMakeLists.txt`          | ✅ 完成 | C++20、GLOB_RECURSE 收集 src/*.cpp、多编译器警告选项                                                       |

### 已知问题

1. ~~Bus.cpp 地址解码 bug~~ ✅ 已修复（PPU 0x2000-0x3FFF / APU 0x4000-0x4017 已正确区分）
2. ~~PPU.cpp 寄存器偏移常量命名错位~~ ✅ 已修复（$2003=OAMADDR、$2005=PPUSCROLL、$2006=PPUADDR、$2007=PPUDATA）
3. ~~Bus 未连接 PPU~~ ✅ 已修复（Bus 有 ppu 成员 + connect_ppu，寄存器读写已转发）
4. ~~CPU.cpp `REL()`~~ ✅ 已修复（commit d8ed3c1，`addr_rel` + `addr_rel |= 0xFF00`）
5. ~~CPU.cpp `IND()`~~ ✅ 已修复（同上，`| p_lo`）
6. **教训（仍有效）**：这类逻辑 bug 在语法/类型层面合法，`-Werror -Wall -Wextra -Wconversion` 均拦不住，只能靠人工 review。
7. **待修：CMakeLists.txt 的警告选项没有生效** —— `add_compile_options()` 在第 40 行调用，而 `add_executable()` 在第 14 行；CMake 的 `add_compile_options` 只影响其后创建的 target，`flags.make` 里实际只有 `-g -O0 -DDEBUG -std=c++20`（Release 分支的 `-flto` 同样失效）。真正打开这些旗标后，现有代码会报：Mapper0.h:25/27 未使用参数（每个编译单元 4 条）、CPU.cpp:275/286 与 PPU.cpp:48 的 `int → u16` 转换。

### 下一步

1. ~~先修 REL/IND 的 3 个逻辑 bug~~ ✅ 已完成（commit d8ed3c1）
2. **实现全部指令函数体 + reset/step/nmi/irq**（大工程，教学模式推进）——链接目前失败于此，是必经之路。用户已理解寻址模式与底层概念，可进入此阶段；教学法：讲 1-2 条的原理（如 LDA/STA/JMP，含标志位 Z/N 的设立规则）+ 用户手写同类（ZCode 不代写，见下方 ⛔ 规则）
   - 当前进度：19 条已实现（LDA/LDX/LDY、STA/STX/STY、TAX/TAY/TXA/TYA/TSX/TXS、SEC/CLC/SED/CLD/SEI/CLI/CLV）；待补 37 条 + XXX + reset/step/nmi/irq
   - 已讲待写：逻辑运算家族 AND/ORA/EOR、比较家族 CMP/CPX/CPY（含 C=无借位 的语义）
3. 实现 APU 和 DMA
4. 之后接 PPU 渲染、主循环、DMA 等

## 教学模式约定 ⭐

- **⛔ 硬性规则（用户明确要求）：禁止任何"非用户手写"的代码。** ZCode 不得 Edit/Write 任何源码文件（.cpp/.h/CMakeLists 等），**连"示范代码"也不行**；也不得在回答里给出可直接复制粘贴的完整函数实现——那等同于代写。ZCode 只做四件事：讲原理与心智模型、给思考题和提示、回答用户提问、在用户写完后做编译与静态核查。违反此规则时用户会回退改动。
- **工作方式：所有代码由用户自己动手写**：讲清原理，留同类练习让用户自己写，不要直接全部实现
- 已讲过的概念：mode()/oper() 分离设计（寻址模式函数只负责算 addr_abs 并推进 pc）、小端序、零页回绕、间接寻址（IZX 三步取址）、跨页加周期
- **⚠️ 教学法转变（因用户"写了大量代码仍不理解"而触发）**：不再给"照抄 x 换 y"式模板，改讲底层心智模型。核心：一个寻址模式回答**同一个问题**——"如何根据指令字节 + 寄存器算出 operand 真正所在的内存地址？"，结果统一落到 `addr_abs`。配合 CPU 的"取指-译码-执行"循环解释每个内部变量（opcode/pc/fetched/addr_abs/addr_rel）。放慢节奏，每步确认理解后再前进。
- **✅ 本阶段已让用户真正掌握（用户主动表示懂了）**：十六进制/字节/bit 换算（0xFF=255、1字节=2个hex位）、按位与 `&`、`& 0xFF` 取低字节 vs `& 0xFF00` 取高字节、`(hi<<8)|lo`/`<<8` 拼 16 位地址（×256=×16²）、PC 作为前移指针（`pc++` 后缀=先读后挪）、**寻址模式统一框架**（直接vs间接、加X/Y、页=256字节、IZX先加后读/IZY先读后加、跨页加周期）。用户现已能自行推导任意寻址模式，不再靠背。

## 架构约定

- 类型：u8/u16/u32/u64（定义在 basic/types.h）
- C++20 标准，禁用编译器扩展（`CMAKE_CXX_EXTENSIONS OFF`）
- 命名：成员变量 snake_case，类 PascalCase
- 构造函数使用初始化列表（参考 Bus 风格）
- Bus 是中央枢纽，连接 CPU、PPU、Cartridge
- ~~codegraph sync~~ 本机无 `cpp/` 子目录，此约定暂不适用
- **构建（用户在 CLion 里构建）**：CLion 自带。cmake 在 `...\CLion\bin\cmake\win\x64\bin\cmake.exe`，MinGW `g++.exe` 在 `...\CLion\bin\mingw\bin\`（g++ 15.2.0）。两目录加进 PATH 后执行：
  `cmake -S <root> -B build_debug -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=<mingw>\g++.exe && cmake --build build_debug`
  当前结果：**编译全过，链接因指令函数体未实现而失败（预期）**。注意：过去记为"无告警"是因为警告旗标根本没生效（见已知问题 7）。
- **ZCode 侧的编译核查通道**：ZCode 的 shell 里找不到上述 CLion 工具链，但 WSL 的 Debian 发行版里有 g++ 14.2.0 + cmake 3.31.6，可直接用来核查用户手写的代码（构建目录放 WSL 侧 `~/nes_build`，不污染仓库）：
  `MSYS_NO_PATHCONV=1 wsl.exe -d Debian -- bash -lc 'cd /mnt/d/devfiles/NesProJects && g++ -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-unused-parameter -Iinclude -Isrc -c src/CPU.cpp -o /tmp/cpu.o'`
