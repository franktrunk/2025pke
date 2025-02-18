/*
 * Utility functions for process management. 
 *
 * Note: in Lab1, only one process (i.e., our user application) exists. Therefore, 
 * PKE OS at this stage will set "current" to the loaded user application, and also
 * switch to the old "current" process after trap handling.
 */

#include "riscv.h"
#include "strap.h"
#include "config.h"
#include "process.h"
#include "elf.h"
#include "string.h"

#include "spike_interface/spike_utils.h"

//Two functions defined in kernel/usertrap.S
extern char smode_trap_vector[]; // S 模式陷阱处理向量，定义在汇编代码中
extern void return_to_user(trapframe*); // 返回用户模式函数

// current points to the currently running user-mode application.
// 当前正在运行的用户模式应用程序指针
process* current = NULL;

//
// switch to a user-mode process
//
void switch_to(process* proc) {
  assert(proc); // 确保目标进程指针非空
  current = proc; // 更新当前运行的进程为目标进程

  // write the smode_trap_vector (64-bit func. address) defined in kernel/strap_vector.S
  // to the stvec privilege register, such that trap handler pointed by smode_trap_vector
  // will be triggered when an interrupt occurs in S mode.
  write_csr(stvec, (uint64)smode_trap_vector);
  // 设置陷阱帧的值（存储在进程结构体中），以便 smode_trap_vector 在进程下次进入内核时使用
  // set up trapframe values (in process structure) that smode_trap_vector will need when
   // 设置进程的内核栈指针
  // the process next re-enters the kernel.
  proc->trapframe->kernel_sp = proc->kstack;  // process's kernel stack
  // 设置内核陷阱处理函数地址
  proc->trapframe->kernel_trap = (uint64)smode_trap_handler;

  // SSTATUS_SPP and SSTATUS_SPIE are defined in kernel/riscv.h
  // set S Previous Privilege mode (the SSTATUS_SPP bit in sstatus register) to User mode.
  unsigned long x = read_csr(sstatus);
  x &= ~SSTATUS_SPP;  // clear SPP to 0 for user mode 清除 SPP 位，将模式设置为用户模式
  x |= SSTATUS_SPIE;  // enable interrupts in user mode 启用用户模式中的中断

  // write x back to 'sstatus' register to enable interrupts, and sret destination mode.
   // 将 x 写回 sstatus 寄存器，启用中断并设置 sret 的目标模式
  write_csr(sstatus, x);

  // set S Exception Program Counter (sepc register) to the elf entry pc.
  // 将 S 异常程序计数器（sepc 寄存器）设置为 ELF 的入口地址
  write_csr(sepc, proc->trapframe->epc);

  // return_to_user() is defined in kernel/strap_vector.S. switch to user mode with sret.
 // return_to_user() 定义在 kernel/strap_vector.S，通过 sret 切换到用户模式
  return_to_user(proc->trapframe);
