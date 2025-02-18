/*
 * Utility functions for trap handling in Supervisor mode.
 */

#include "riscv.h"
#include "process.h"
#include "strap.h"
#include "syscall.h"

#include "spike_interface/spike_utils.h"

//
// handling the syscalls. will call do_syscall() defined in kernel/syscall.c
//处理系统调用（syscall）。调用在 kernel/syscall.c 中定义的 do_syscall()。
//这个函数用于处理用户程序发出的系统调用请求。
static void handle_syscall(trapframe *tf) {
  // tf->epc points to the address that our computer will jump to after the trap handling.
  // tf->epc 指向处理完陷阱后，计算机会跳转到的地址。
  // for a syscall, we should return to the NEXT instruction after its handling.
  // 对于系统调用，我们应该返回到系统调用处理后的下一条指令。
  // 在 RV64G 中，每条指令占用 32 位（即 4 字节）。
  // in RV64G, each instruction occupies exactly 32 bits (i.e., 4 Bytes)
  tf->epc += 4;

  // TODO (lab1_1): remove the panic call below, and call do_syscall (defined in
  // kernel/syscall.c) to conduct real operations of the kernel side for a syscall.
  // IMPORTANT: return value should be returned to user app, or else, you will encounter
  // problems in later experiments!
   
}

//
// kernel/smode_trap.S will pass control to smode_trap_handler, when a trap happens
// in S-mode.
//
void smode_trap_handler(void) {
  // make sure we are in User mode before entering the trap handling.
  // 确保我们处于用户模式（User mode）之前进入陷阱处理。
  // we will consider other previous case in lab1_3 (interrupt).
  if ((read_csr(sstatus) & SSTATUS_SPP) != 0) panic("usertrap: not from user mode");

  assert(current);
  // save user process counter.
   // 保存用户进程的程序计数器（epc），即发生陷阱时的执行位置。
  current->trapframe->epc = read_csr(sepc);

  // if the cause of trap is syscall from user application.
  // read_csr() and CAUSE_USER_ECALL are macros defined in kernel/riscv.h
  if (read_csr(scause) == CAUSE_USER_ECALL) {
    handle_syscall(current->trapframe);
  } else {
    sprint("smode_trap_handler(): unexpected scause %p\n", read_csr(scause));
    sprint("            sepc=%p stval=%p\n", read_csr(sepc), read_csr(stval));
    panic( "unexpected exception happened.\n" );
  }

  // continue (come back to) the execution of current process.
   // 恢复当前进程的执行（即返回当前进程继续执行）。
  switch_to(current);// 切换回当前进程，继续其执行。
}
