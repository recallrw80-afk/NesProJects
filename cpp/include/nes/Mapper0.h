//
// Created by local on 2026/5/25.
//

#ifndef NES_MAPPER0_H
#define NES_MAPPER0_H
#include <basic/types.h>
#include <nes/Mapper.h>

class Mapper0 : public Mapper{
private:
    u8 prg_bank;
    u8 chr_bank;

public:
    Mapper0(u8 prg,u8 chr) : prg_bank(prg), chr_bank(chr) {}

    size_t cpu_map_read(u16 address) override {
        // 0x8000-0xFFFF -> 若 1 bank : mirror; 若 2 bank: 直接映射
        if (prg_bank == 1)
            return (address - 0x8000) & 0x3FFF; // 16KB mirror
        return address - 0x8000;
    }

    void cpu_map_write(u16 address, size_t value) override {}
    size_t ppu_map_read(u16 address) override {return address;}
    void ppu_map_write(u16 address, u8 value) override {};
};

#endif //NES_MAPPER0_H
