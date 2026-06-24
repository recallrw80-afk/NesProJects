#include <string>
#include <iostream>
#include "Cartridge.h"
#include <ios>
#include <fstream>


/*
 * .nes 文件结构
 * 字节偏移    大小       字段
  ────────────────────────────────────────
  0x00       4          Magic: 4E 45 53 1A ("NES" + 0x1A)
  0x04       1          PRG-ROM bank 数量（每个 bank = 16384 字节 = 16KB）
  0x05       1          CHR-ROM bank 数量（每个 bank = 8192 字节 = 8KB）
  0x06       1          Flags 6
  0x07       1          Flags 7
  0x08       8          保留（填 0）
  0x10       512        可选：Trainer（只有 Flags6 bit2=1 时才存在）
  0x10+?     N          主体：PRG-ROM 数据 + CHR-ROM 数据

  Flags 6 每位的含义

  Bit  7  6  5  4  3  2  1  0
       │  │  │  │  │  │  │  │
       │  │  │  │  │  │  │  └── Mirroring: 0=水平, 1=垂直
       │  │  │  │  │  │  └──── Battery/RAM: 1=卡带有电池存档
       │  │  │  │  │  └─────── Trainer: 1=后面有512字节Trainer
       │  │  │  │  └────────── 忽略（Four-screen mirroring）
       └──┴──┴──┘───────────── Mapper ID 的低 4 位

    Flags 7 的含义
      Bit  7  6  5  4  3  2  1  0
           │  │  │  │  └──┴──┴──┴── 未使用/保留
           └──┴──┴──┘────────────── Mapper ID 的高 4 位
 */
// 16384：每个 PRG bank = 16KB = 16 × 1024
constexpr size_t PRG_BANK_SIZE = 16384;
// iNES 头部大小
constexpr size_t HEADER_SIZE = 16;
//Trainer 大小
constexpr size_t TRAINER_SIZE = 512;
// CHR-RAM 默认大小（8KB）
constexpr size_t CHR_RAM_SIZE = 8192;

bool Cartridge::load(const std::string& filename)
{
    // 1.读取文件到 buffer
    // 二进制打开文件
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        return false;

    // 创建迭代器 std::vec 从开始读取到最后 存储到 vec<u8> 中
    const std::vector<u8> buffer(std::istreambuf_iterator<char>(file),{});
    // 2.验证  INES 头部
    if (buffer.size() < HEADER_SIZE )
        return false;

    if (buffer[0] != 'N' || buffer[1] != 'E' || buffer[2] != 'S' || buffer[3] != 0x1A)
        return false;

    // 3.解析头部
    //PRG-ROM 数量
    prg_banks_count = buffer[4];
    //CHR-ROM 数量
    chr_banks_count = buffer[5];
    const u8 flags6 = buffer[6];
    const u8 flags7 = buffer[7];

    //设置镜像模式
    //提取 bit
    mirroring = (flags6 & 0x01) ? VERTICAL : HORIZONTAL;
    battery_ram = (flags6 & 0x02);
    trainer = (flags6 & 0x04);

    // Mapper Id
    mapper_id = (flags7 & 0xF0) | (flags6 >> 4);

    //计算 ROM 数据起始偏移
    size_t rom_offset = HEADER_SIZE;
    if ( trainer )
        rom_offset += TRAINER_SIZE;

    // 计算 PRG 和 CHR 数据大小
    const size_t prg_size = prg_banks_count * PRG_BANK_SIZE;
    const size_t char_size = chr_banks_count * CHR_RAM_SIZE;

    // 越界检查
    if (buffer.size() < rom_offset + prg_size + char_size)
        return false;

    //读取 PRG-ROM
    prg_rom = std::vector<u8>(
        buffer.begin() + rom_offset,
        buffer.begin() + rom_offset + prg_size
    );

    //读取 CHR—ROM
    if (chr_banks_count == 0)
    {
        //8kb CHR-RAM 初始化为0
        chr_rom.resize(CHR_RAM_SIZE,0);
    }
    else
    {
        chr_rom = std::vector<u8>(
            buffer.begin() + rom_offset + prg_size,
            buffer.begin() + rom_offset + prg_size + char_size
        );
    }

    return true;
}

void Cartridge::print_info() const
{
    std::cout << "=== Cartridge Info ===" << std::endl;
    std::cout << "Mapper ID:       " << static_cast<int>(mapper_id) << std::endl;
    std::cout << "PRG Banks:       " << static_cast<int>(prg_banks_count) << " (" << prg_banks_count * 16 << " KB)" << std::endl;
    std::cout << "CHR Banks:       " << static_cast<int>(chr_banks_count) << " (" << chr_banks_count * 8 << " KB)" << std::endl;
    std::cout << "Mirroring:       " << (mirroring ? "Vertical" : "Horizontal") << std::endl;
    std::cout << "Battery RAM:     " << (battery_ram ? "Yes" : "No") << std::endl;
    std::cout << "Trainer:         " << (trainer ? "Yes" : "No") << std::endl;
}