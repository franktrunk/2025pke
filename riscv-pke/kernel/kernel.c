/*
 * Supervisor-mode startup codes
 */

#include "riscv.h"
#include "string.h"
#include "elf.h"
#include "process.h"

#include "spike_interface/spike_utils.h"

// process is a structure defined in kernel/process.h
// 定义了一个进程结构，用于表示用户程序。
process user_app;

//
// load the elf, and construct a "process" (with only a trapframe).
// 加载 ELF 文件，并构造一个仅包含 trapframe 的 "process" 结构。
// load_bincode_from_host_elf is defined in elf.c
// load_bincode_from_host_elf 在 elf.c 文件中定义。
void load_user_program(process *proc) {
  // USER_TRAP_FRAME is a physical address defined in kernel/config.h

  proc->trapframe = (trapframe *)USER_TRAP_FRAME;
  // 将 trapframe 清零，以初始化寄存器状态。
  memset(proc->trapframe, 0, sizeof(trapframe));
  // USER_KSTACK is also a physical address defined in kernel/config.h
   // USER_TRAP_FRAME 是一个物理地址，定义在 kernel/config.h 中。
  proc->kstack = USER_KSTACK;
   // 设置用户栈顶地址。
  proc->trapframe->regs.sp = USER_STACK;
  // 调用函数 load_bincode_from_host_elf，将用户程序的二进制代码从主机 ELF 文件加载到内存中。
  // load_bincode_from_host_elf() is defined in kernel/elf.c
  load_bincode_from_host_elf(proc);
}

//
// s_start: S-mode entry point of riscv-pke OS kernel.
//
int s_start(void) {
  sprint("Enter supervisor mode...\n");// 输出进入 S 模式的信息。
  /*
  注意：在 lab1 中，我们使用直接映射（即 Bare 模式）进行内存管理。这意味着：虚拟地址 = 物理地址。
  因此，目前需要将 satp 设置为 0。我们将在 lab2_x 中启用分页机制。
  */
  // Note: we use direct (i.e., Bare mode) for memory mapping in lab1.
  // which means: Virtual Address = Physical Address
  // therefore, we need to set satp to be 0 for now. we will enable paging in lab2_x.
  // 
  // write_csr is a macro defined in kernel/riscv.h
  // write_csr 是一个宏，定义在 kernel/riscv.h 中，用于写入 CSR 寄存器。
  write_csr(satp, 0);
 //satp 是一种重要的控制与状态寄存器，主要用于控制地址转换（页表）和虚拟内存的启用
  // the application code (elf) is first loaded into memory, and then put into execution
  load_user_program(&user_app);

  sprint("Switch to user mode...\n");
  // switch_to() is defined in kernel/process.c
  // switch_to 函数定义在 kernel/process.c 文件中，用于切换到指定的进程。
  switch_to(&user_app);
 // 理论上，代码不应该执行到这里。
  // we should never reach here.
  return 0;
}
