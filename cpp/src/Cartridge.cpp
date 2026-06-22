#include <string>
#include <iostream>
#include "Cartridge.h"
#include <ios>
#include <fstream>

bool Cartridge::load(const std::string& filename)
{
    // 1.读取文件到 buffer
    // 二进制打开文件
    std::ifstream file(filename, std::ios::binary);
    // 创建迭代器 std::vec 从开始读取到最后 存储到 vec<u8> 中
    std::vector<u8> buffer(std::istreambuf_iterator<char>(file),{});

    // 2.验证 iNES 头部
    if (buffer[0] != 'N' || buffer[1] != 'E' || buffer[2] != 'S' || buffer[3] != 0x1A)
    {
        return false;
    }

    // 3.解析头部
    //PRG-ROM 数量
    prg_banks_count = buffer[4];
    //CHR-ROM 数量
    chr_banks_count = buffer[5];
    u8 flags6 = buffer[6];
    u8 flags7 = buffer[7];

    //设置镜像模式
    mirroring = (flags6 & 0x30) ? VERTICAL : HORIZONTAL;
    battery_ram = (flags6 & 0x02);
    trainer = (flags6 & 0x04);
    mapper_id = (flags7 & 0xF0) | (flags6 >> 4);


    return false;
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