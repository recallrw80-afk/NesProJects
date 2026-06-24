//
// Created by recall on 2026/6/24.
//

#ifndef NES_PPU_H
#define NES_PPU_H
#include <basic/types.h>
#include <nes/Cartridge.h>

class PPU {
private:
    // PPU 内部 RAM：2KB 显存 (VRAM)，用于 nametable
    std::vector<u8> vram;

    // 调色板 RAM：32 字节
    std::vector<u8> palette_ram;

    // 连接的卡带 (用于读取 CHR-ROM/ CHR-RAM)
    Cartridge* cartridge;

    //内部寄存器
    // PPUCTRT 控制器
    u8 ctrl;
    // PPUMASK 掩码
    u8 mask;
    // PPUSTATUS 状态
    u8 status;
    // QAMADDR OAM 地址
    u8 oam_addr;
    // PPUSCROLL 滚动
    u8 scroll_x;
    u8 scroll_y;
    // PPUADDR 地址
    u16 vram_addr;
    // PPUDATA 数据
    u16 vram_data;

    // 暂存器(用于 PPUSCORLL/ PPUADDR的两次写入)
    u8 write_latch;
    bool latch_toggle; //false 第一次写入， ture 第二次写入

    // 内部 VRAM 读缓冲
    u8 read_buffer;

public:
    PPU();
    ~PPU() = default;

    // 连接卡带
    void connect_cartridge(Cartridge* cartridge);

    // CPU 通过 Bus 访问 PPU 寄存器的接口
    // 寄存器读取 (地址已由 Bus 解码为 0x2000-0x2007)
    u8 register_read(u16 address);
    // 寄存器写入
    void register_write(u16 address, u8 value);

    //VRAM 访问 (通过 PPUADDR / PPUDATA)
    u8 vram_read(u16 address);
    void vram_write(u16 address, u8 value);

    // 调色板访问
    u8 palette_read(u16 address);
    void palette_write(u16 address, u8 value);

private:
    // 地址递增(根据 PPUCTRL bit2 决定 +1 还是 +32)
    void increment_vram_addr();
};
#endif //NES_PPU_H
