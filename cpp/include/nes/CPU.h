//
// Created by recall on 2026/6/24.
//

#ifndef NES_CPU_H
#define NES_CPU_H
#include <basic/types.h>
#include <nes/Bus.h>

#include "Bus.h"

class CPU {
private:
    //寄存器
    u8 a;   // 累加器
    u8 x;   // X 变址寄存器
    u8 y;   // Y 变址寄存器
    u16 pc; // 程序计数器
    u8 sp;  // 栈指针 ($0100-$01FF)
    u8 opcode; // 当前操作码

    // 状态标志位(P 寄存器): NV-BDIZC
    bool flag_c;    // bit0 Carry
    bool flag_z;    // bit1 Zero
    bool flag_i;    // bit2 Interrupt Disable
    bool flag_d;    // bit3 Decimal
    bool flag_b;    // bit4 Break
    bool flag_v;    // bit6 OverFlow
    bool flag_n;    // bit7 Negative

    // 总线连接
    Bus* bus;

    // 内部状态
    u16 addr_abs;   //绝对地址
    u16 addr_rel;   //相对地址
    u8 fetched;     //读取到的操作的数值
    u32 cycles;     //总周期计数器

    // 内部辅助方法
    u8  read_mem(u16 addr);
    void write_mem(u16 addr, u8 data);
    u8 fetch();
    void push(u8 value);
    u8 pull();
    u8 get_flags();
    void set_flags(u8 value);

    //寻址模式
    void IMP(); void IMM(); void ZP0(); void ZPX(); void ZPY();
    void ABS(); void ABX(); void ABY(); void IND(); void IZX();
    void IZY(); void REL(); void ACC();

    // 指令操作
    void ADC(); void AND(); void ASL(); void BCC(); void BCS();
    void BEQ(); void BIT(); void BMI(); void BNE(); void BPL();
    void BRK(); void BVC(); void BVS(); void CLC(); void CLD();
    void CLI(); void CLV(); void CMP(); void CPX(); void CPY();
    void DEC(); void DEX(); void DEY(); void EOR(); void INC();
    void INX(); void INY(); void JMP(); void JSR(); void LDA();
    void LDX(); void LDY(); void LSR(); void NOP(); void ORA();
    void PHA(); void PHP(); void PLA(); void PLP(); void ROL();
    void ROR(); void RTI(); void RTS(); void SBC(); void SEC();
    void SED(); void SEI(); void STA(); void STX(); void STY();
    void TAX(); void TAY(); void TSX(); void TXA(); void TXS();

    //未定义操作码
    void XXX();

    struct Instruction {
        void (CPU::*mode)();
        void (CPU::*oper)();
        const char* name;
        u8 cycles;
    };

    static const Instruction table[256];

public:
    CPU();
    ~CPU() = default;

    void connect_bus(Bus* bus_ptr);

    void reset();
    void step();
    void nmi();
    void irq();

    u32 get_cycles() const;
};

#endif //NES_CPU_H
