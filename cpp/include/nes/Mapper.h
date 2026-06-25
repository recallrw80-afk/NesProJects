//
// Created by local on 2026/5/25.
//

#ifndef NES_MAPPER_H
#define NES_MAPPER_H

#include <basic/types.h>

class Mapper
{
public:
    virtual ~Mapper() = default;

    virtual u16  cpu_map_read(u16 address) = 0;
    virtual void    cpu_map_write(u16 address, u8 value) = 0;
    virtual u16  ppu_map_read(u16 address) = 0;
    virtual void    ppu_map_write(u16 address, u8 value) = 0;
};

#endif //NES_MAPPER_H
