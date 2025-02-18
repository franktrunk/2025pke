#ifndef _PROC_H_
#define _PROC_H_

#include "riscv.h"
//定义陷阱帧（trapframe）的结构体，用于保存用户态进程的上下文信息
// 包括寄存器状态、内核栈指针、陷阱处理函数指针和用户态的程序计数器。
typedef struct trapframe_t {
  // space to store context (all common registers)
  //用于保存寄存器上下文（所有通用寄存器）
  /* offset:0   */ riscv_regs regs;

  
  // process's "user kernel" stack
   // 进程的“用户内核”栈指针
  /* offset:248 */ uint64 kernel_sp;
  // pointer to smode_trap_handler
  // 指向 S 模式陷阱处理函数的指针
  /* offset:256 */ uint64 kernel_trap;
  // saved user process counter
  // 保存用户态的程序计数器（EPC: Exception Program Counter）
  /* offset:264 */ uint64 epc;
}trapframe;

// the extremely simple definition of process, used for begining labs of PKE
typedef struct process_t {
  // pointing to the stack used in trap handling.
   // 指向用于陷阱处理的内核栈。
  uint64 kstack;
  // trapframe storing the context of a (User mode) process.
  trapframe* trapframe;
  // 指向存储用户态进程上下文的陷阱帧。
}process;

void switch_to(process*);
// 指向当前正在运行的进程结构体。
extern process* current;

#endif
