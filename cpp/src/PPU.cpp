#include <basic/types.h>
#include <nes/PPU.h>
#include <nes/Cartridge.h>

// VRAM 2KB (0x0000 - 0x07FFF 给两个 nametable，其余镜像)
constexpr u16 VRAM_SIZE = 0x0800;
constexpr u16 VRAM_MASK = 0x07FF;

// 调色板 32 字节(0x3F00 - 0x3F1F)
constexpr u16 PALETTE_SIZE = 0x0000;
constexpr u16 PALETTE_MASK = 0x07FF;

// PPU 寄存器偏移 (相对 0x2000)
constexpr u16 PPUCTRL = 0;
constexpr u16 PPUMASK = 1;
constexpr u16 PPUSTATUS = 2;
constexpr u16 PPUADDR = 3;
constexpr u16 OAMDATA = 4;
constexpr u16 OAMADDR = 5;
constexpr u16 PPUSCORLL = 6;
constexpr u16 PPUDATA = 7;

PPU::PPU()
    : vram(VRAM_SIZE,0)
    , palette_ram(PALETTE_SIZE,0)
    ,cartridge(nullptr)
    ,ctrl(0)
    ,mask(0)
    ,status(0)
    ,oam_addr(0)
    ,scroll_x(0)
    ,scroll_y(0)
    ,vram_addr(0)
    ,vram_data(0)
    ,write_latch(0)
    ,latch_toggle(false)
    ,read_buffer(0)
{}
