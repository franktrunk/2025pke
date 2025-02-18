#ifndef _CONFIG_H_
#define _CONFIG_H_

// we use only one HART (cpu) in fundamental experiments
#define NCPU 1
// 处理器数量为1
#define DRAM_BASE 0x80000000
// 系统内存固定基地址
/* we use fixed physical (also logical) addresses for the stacks and trap frames as in
 Bare memory-mapping mode */
// user stack top
#define USER_STACK 0x81100000
// 用户栈的起始地址
// the stack used by PKE kernel when a syscall happens
#define USER_KSTACK 0x81200000
// 内核栈的地址
// the trap frame used to assemble the user "process"
#define USER_TRAP_FRAME 0x81300000
// 用户陷阱帧的地址。陷阱帧用于保存程序的上下文，系统调用或者中断的时候便于恢复
#endif
