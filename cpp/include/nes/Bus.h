//
// Created by recall on 2026/6/24.
//

#ifndef NES_BUS_H
#define NES_BUS_H
#include <basic/types.h>
#include <nes/CPU.h>
#include <nes/Cartridge.h>

/*
Bus 是连接一切的枢纽，NRE CPU 地址空间布局：
    0x0000 - 0x1FFF  → CPU RAM (2KB, 镜像 4 次)
    0x2000 - 0x3FFF  → PPU 寄存器 (8 字节, 镜像 1024 次)
    0x4000 - 0x4017  → APU + I/O 寄存器
    0x4020 - 0xFFFF  → 卡带 (PRG-ROM / Mapper)
*/

class Bus
{
private:
    //CPU 内置 RAM: 2KB (0x0000 - 0x07FF, 其余到 0x1FFF 是镜像)
    std::vector<u8> cpu_ram;
    // 连接的设备
    Cartridge* cartridge;
    CPU* cpu;

public:
    Bus(); //构造函数
    ~Bus() = default; //析构函数

    // 插入卡带
    void insert_cartridge(Cartridge *cart);
    void connect_cpu(CPU *cpu_ptr);

    // CPU 侧读写（整个 64kb 地址空间）
    u8 cpu_read(u16 address);
    void cpu_write(u16 address, u8 value);

    // 调试
    Cartridge* get_cartridge() const { return cartridge; }
};

#endif //NES_BUS_H
