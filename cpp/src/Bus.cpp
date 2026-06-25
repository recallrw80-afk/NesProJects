#include <basic/types.h>
#include <nes/Bus.h>

// 地址掩码常量
// CPU RAM: 2KB(0x0000 - 0x07FF),其余到 0x1FFF 是镜像
constexpr u16 CPU_RAM_SIZE = 0x0800; // 2048 bytes
constexpr u16 CPU_RAM_MASK = 0x07FF; //2kb 掩码 用于镜像

// PPU 寄存器范围
constexpr u16 PPU_REG_BASE = 0x2000;
constexpr u16 PPU_REG_END = 0x3FFF;

// APU + I/O 寄存器范围
constexpr u16 APU_IO_BASE = 0x4000;
constexpr u16 APU_IO_END = 0x4017;

// 卡带地址范围起始
constexpr u16 CART_BASE = 0x4020;

// Bus::Bus() {
//     cpu_ram = std::vector<u8>(CPU_RAM_SIZE, u8(0));
//     cartridge = nullptr;
//     cpu = nullptr;
// }
/*
效率更高（直接构造）
对所有成员都适用
风格统一
避免以后成员改成 const、引用、无默认构造类型时重构代码
 */
Bus::Bus()
    : cpu_ram(CPU_RAM_SIZE, 0)
      , cartridge(nullptr)
      , cpu(nullptr)
      , ppu(nullptr) {
}

void Bus::insert_cartridge(Cartridge *cart) {
    cartridge = cart;
}

void Bus::connect_cpu(CPU *cpu_ptr) {
    cpu = cpu_ptr;
}

void Bus::connect_ppu(PPU *ppu_ptr) {
    ppu = ppu_ptr;
}

u8 Bus::cpu_read(u16 address) {
    //0x0000 - 0x1FFFF
    if (address <= 0x1FFF)
        return cpu_ram[address & CPU_RAM_MASK];

    // 0x2000 - 0x3FFF: PPU 寄存器 (8 字节, 镜像 1024 次)
    if (address >= PPU_REG_BASE && address <= PPU_REG_END)
        return ppu ? ppu->register_read(address) : 0;

    // 0x4000 - 0x4017: APU + I/O 寄存器
    // TODO: 连接到 APU/手柄 后进行转发
    if (address >= APU_IO_BASE && address <= APU_IO_END)
        return 0;

    // 0x4020 - 0xFFFF: 卡带 (PRG-ROM / Mapper)
    if (address >= CART_BASE && cartridge != nullptr)
        return cartridge->cpu_read(address);

    return 0;
}

void Bus::cpu_write(u16 address, u8 value) {
    //0x0000 - 0x1FFF: CPU RAM
    if (address <= 0x1FFF) {
        cpu_ram[address & CPU_RAM_MASK] = value;
        return;
    }

    // 0x2000 - 0x3FFF: PPU 寄存器
    // TODO: 连接到 PPU 后进行转发
    if (address >= PPU_REG_BASE && address <= PPU_REG_END) {
        if (ppu) ppu->register_write(address, value);
        return;
    }

    // 0x4000 - 0x4017: APU + I/O 寄存器
    // TODO: 连接到 APU/手柄 后进行转发
    if (address >= APU_IO_BASE && address <= APU_IO_END)
        return;

    // 0x4020 - 0xFFFF: 卡带 (Mapper 寄存器 / SRAM)
    if (address >= CART_BASE && cartridge != nullptr)
        cartridge->cpu_write(address, value);
}
