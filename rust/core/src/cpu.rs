
pub struct CPU {
    pub pc: u16, //寄存器
    pub acc: u8 //累加器
}

pub enum Instruction {
    NOP, //按键触发 空操作
    INC //按键触发 
}
