//
// Created by local on 2026/5/25.
//

#ifndef NES_CARTIDGE_H
#define NES_CARTIDGE_H
#include <vector>
#include <basic/types.h>
#include <nes/Mapper.h>
#include <nes/Mapper0.h>

// 定义枚举，表示镜像模式
enum Mirroring
{
    VERTICAL,   //编译器默认赋值为0
    HORIZONTAL  //编译器默认赋值为1
};

class Cartridge
{
    private:
        std::vector<u8> prg_rom; //程序ROM
        std::vector<u8> chr_rom; //图形ROM
        std::vector<u8> sram; //卡带存档 RAM
        Mapper* mapper;

        u16 mapper_id;
        u8 prg_banks_count;        //16KB bank 数量
        u8 chr_banks_count;        //8KB bank 数量
        Mirroring mirroring;     //水平或垂直镜像
        bool battery_ram;   //是否有存档
        bool trainer;       //512字节训练器
    public:
        Cartridge() : mapper(nullptr) {}
        ~Cartridge() { delete mapper; }

        bool load(const std::string& filename);

        // CPU 通过总线读取 PRC-rom
        u8 cpu_read(u16 address);
        void cpu_write(u16 address, u8 value);

        // PPU 读取 CHR-ROM
        u8 ppu_read(u16 address);
        void ppu_write(u16 address, u8 value);

        // 获取 mepper_id
        u16 get_mepper_id() const { return mapper_id; }

        // 打印当前信息
        void print_info() const;
};

#endif //NES_CARTIDGE_H
