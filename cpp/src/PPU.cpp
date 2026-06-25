#include <basic/types.h>
#include <nes/PPU.h>
#include <nes/Cartridge.h>

// VRAM 2KB (0x0000 - 0x07FFF 给两个 nametable，其余镜像)
constexpr u16 VRAM_SIZE = 0x0800;
constexpr u16 VRAM_MASK = 0x07FF;

// 调色板 32 字节(0x3F00 - 0x3F1F)
constexpr u16 PALETTE_SIZE = 32;
constexpr u16 PALETTE_MASK = 0x001F;

// constexpr u16 PPU_REF_MASK = 0x2007; //8 字节, 镜像 1024次

// PPU 寄存器偏移 (相对 0x2000)
constexpr u16 PPUCTRL = 0; // $2000 - 控制器
constexpr u16 PPUMASK = 1; // $2001 - 掩码
constexpr u16 PPUSTATUS = 2; // $2002 - 状态
constexpr u16 OAMADDR = 3; // $2003 - OAM 地址
constexpr u16 OAMDATA = 4; // $2004 - OAM 数据
constexpr u16 PPUSCROLL = 5; // $2005 - 滚动
constexpr u16 PPUADDR = 6; // $2006 - 地址
constexpr u16 PPUDATA = 7; //$2007 - 数据

PPU::PPU()
    : vram(VRAM_SIZE, 0)
      , palette_ram(PALETTE_SIZE, 0)
      , cartridge(nullptr)
      , ctrl(0)
      , mask(0)
      , status(0)
      , oam_addr(0)
      , scroll_x(0)
      , scroll_y(0)
      , vram_addr(0)
      // , vram_data(0)
      , write_latch(0)
      , latch_toggle(false)
      , read_buffer(0) {
}

void PPU::connect_cartridge(Cartridge *cart) {
    cartridge = cart;
}

void PPU::increment_vram_addr() {
    // PPUCTRL bit2: 0 = 每次 +1, 1 = 每次 +32
    vram_addr += (ctrl & 0x04) ? 32 : 1;
}

u8 PPU::register_read(u16 address) {
    // Bus 传入的 address 已经是 0x2000-0x2007 范围（经过镜像解码后）
    // 但这里我们直接取低 3 位作为寄存器索引
    u16 reg = address & 0x07;

    switch (reg) {
        case PPUSTATUS: {
            // 读取状态寄存器：bit7 = VBlank, 读取后清除 VBlank 标志
            u8 result = status;
            status &= ~0x80; // 清除 bit7 (VBlank)
            latch_toggle = false; // 读取 STATUS 重置写锁存到 first-write 状态
            return result;
        }
        case OAMADDR:
            // OAM 数据读取 — OAM 未实现，暂返回 0
            return 0;
        case PPUDATA: {
            // 读取 VRAM 数据
            // 注意：有 1 字节的缓冲延迟（除调色板外）
            u8 result = read_buffer;

            //从当前地址读取 有效地址范围 0x0000-0x3FFF
            u16 addr = vram_addr & 0x3FFF; //取低14位（页内偏移）
            if (addr < 0x2000) {
                // CHR-ROM/ CHR-RAM
                if (cartridge != nullptr)
                    read_buffer = cartridge->ppu_read(addr);
                else
                    read_buffer = 0;
            } else if (addr < 0x3F00) {
                // Nametable VRAM (0x2000-0x2FFF, 镜像)
                read_buffer = vram[addr & VRAM_MASK];
            } else {
                // Palette RAM (0x3F00-0x3FFF)
                u8 palette_addr = (addr & PALETTE_MASK);
                // 镜像: 0x3F10, 0x3F14, 0x3F18, 0x3F1C → 0x3F00, 0x3F04, 0x3F08, 0x3F0C
                if (palette_addr >= 16 && (palette_addr & 0x03) == 0)
                    palette_addr -= 16;
                result = palette_ram[palette_addr]; // 立即返回调色板数据
                read_buffer = palette_ram[palette_addr]; // 缓冲填入"下方的"nametable 数据
            }
            // 地址递增
            increment_vram_addr();
            return result;
        }
        default:
            // PPUCTRL, PPUMASK, OAMADDR, PPUSCROLL, PPUADDR 通常不可读
            return 0;
    }
    return 0;
}

void PPU::register_write(u16 address, u8 value) {
    u16 reg = address & 0x07;
    switch (reg) {
        case PPUCTRL: {
            ctrl = value;
            // bit1-0 决定 nametable 基地址，写入到内部暂存
            // 这里暂不处理完整的 VRAM 地址逻辑
            break;
        }

        case PPUMASK:
            mask = value;
            break;

        case OAMADDR:
            oam_addr = value;
            break;

        case OAMDATA:
            //  OAM 数据写入 — OAM 未实现
            break;

        case PPUSCROLL: {
            if (!latch_toggle) {
                // 第一次写 x 滚动
                scroll_x = value;
            } else {
                // 第二次写：Y 滚动
                scroll_y = value;
            }
            latch_toggle = !latch_toggle;
            break;
        }

        case PPUADDR: {
            if (!latch_toggle) {
                write_latch = value;
            } else {
                // 第二次写：低字节，组合成完整地址
                vram_addr = (static_cast<u16>(write_latch) << 8) | value;
            }
            latch_toggle = !latch_toggle;
            break;
        }

        case PPUDATA: {
            // 通过 PPUDATA 写入 VRAM
            u16 addr = vram_addr & 0x3FFF;
            if (addr < 0x2000) {
                // CHR-ROM / CHR-RAM 写入
                if (cartridge != nullptr)
                    cartridge->ppu_write(addr, value);
            } else if (addr < 0x3F00) {
                // Nametable VRAM
                vram[addr & VRAM_MASK] = value;
            } else {
                // Palette RAM
                u8 palette_addr = (addr & PALETTE_MASK);
                if (palette_addr >= 16 && (palette_addr & 0x03) == 0)
                    palette_addr -= 16;
                palette_ram[palette_addr] = value;
            }
            increment_vram_addr();
            break;
        }

        default:
            break;
    }
}
