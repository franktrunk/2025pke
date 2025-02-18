/*
 * Machine-mode C startup codes
 */
 // 本文件定义了RISC-V 机器上运行的代理内核的机器模式入口点和初始化逻辑
#include "util/types.h"
#include "kernel/riscv.h"
#include "kernel/config.h"
#include "spike_interface/spike_utils.h"

//
// global variables are placed in the .data section.
// stack0 is the privilege mode stack(s) of the proxy kernel on CPU(s)
// allocates 4KB stack space for each processor (hart)
//
// NCPU is defined to be 1 in kernel/config.h, as we consider only one HART in basic
// labs.
// 全局变量放在.data 部分。stack0 是 CPU 上的代理内核的特权模式堆栈.
// 为每个处理器分配(heart) 4KB 堆栈空间 
// 在 kernel/config.h 中，NCPU 定义为 1，所以只分配一个栈
//  这行代码的作用是分配一个全局数组 stack0，大小为 4096 * NCPU 字节，并确保它的起始地址对齐到 16 字节边界。它是用于为每个处理器（hart）分配 4KB 的栈空间。
__attribute__((aligned(16))) char stack0[4096 * NCPU];


// 外部函数声明

// sstart() is the supervisor state entry point defined in kernel/kernel.c
// `s_start`: Supervisor 模式的入口点，定义在 kernel/kernel.c 中
extern void s_start();

// htif is defined in spike_interface/spike_htif.c, marks the availability of HTIF
// `htif`: 表示 Host-Target 接口（HTIF）的可用性，定义在 spike_interface/spike_htif.c 中
// HTIF 是 Host-Target Interface（宿主机-目标机接口）的缩写。它是 RISC-V 模拟器 Spike 提供的一种接口，用于在模拟的目标平台（Target）和宿主平台（Host）之间进行通信和数据传递
extern uint64 htif;
// g_mem_size is defined in spike_interface/spike_memory.c, size of the emulated memory
// `g_mem_size`: 模拟内存的大小，定义在 spike_interface/spike_memory.c 中
extern uint64 g_mem_size;

//
// get the information of HTIF (calling interface) and the emulated memory by
// parsing the Device Tree Blog (DTB, actually DTS) stored in memory.
//
// the role of DTB is similar to that of Device Address Resolution Table (DART)用于描述硬件
// in Intel series CPUs. it records the details of devices and memory of the
// platform simulated using Spike.
//
void init_dtb(uint64 dtb) {
  // defined in spike_interface/spike_htif.c, enabling Host-Target InterFace (HTIF)
  // 使用DTB启用接口
  query_htif(dtb);
  if (htif) sprint("HTIF is available!\r\n");

  // defined in spike_interface/spike_memory.c, obtain information about emulated memory
 // 使用DTB获取模拟内存信息
  query_mem(dtb);
  sprint("(Emulated) memory size: %ld MB\n", g_mem_size >> 20);
}

//
// delegate (almost all) interrupts and most exceptions to S-mode.
// after delegation, syscalls will handled by the PKE OS kernel running in S-mode.
//将几乎所有的中断和大部分异常委托给 S 模式处理，委托之后，系统调用将由运行在 S 模式的 PKE OS 内核处理。
static void delegate_traps() {
   // 确认处理器是否支持 S 模式。如果不支持，则终止。
  // supports_extension macro is defined in kernel/riscv.h
  if (!supports_extension('S')) {
    // confirm that our processor supports supervisor mode. abort if it does not.
    sprint("S mode is not supported.\n");
    return;
  }

  // 定义需要委托的中断和异常，宏定义在
  // macros used in following two statements are defined in kernel/riscv.h
  uintptr_t interrupts = MIP_SSIP | MIP_STIP | MIP_SEIP;// 中断类型
  uintptr_t exceptions = (1U << CAUSE_MISALIGNED_FETCH) | (1U << CAUSE_FETCH_PAGE_FAULT) |
                         (1U << CAUSE_BREAKPOINT) | (1U << CAUSE_LOAD_PAGE_FAULT) |
                         (1U << CAUSE_STORE_PAGE_FAULT) | (1U << CAUSE_USER_ECALL);

  // writes 64-bit values (interrupts and exceptions) to 'mideleg' and 'medeleg' (two
  // priviledged registers of RV64G machine) respectively.
  //
  // write_csr and read_csr are macros defined in kernel/riscv.h
   // 将中断和异常写入机器模式特权寄存器 `mideleg` 和 `medeleg` 中
  write_csr(mideleg, interrupts);
  write_csr(medeleg, exceptions);
   // 确认写入是否成功
  assert(read_csr(mideleg) == interrupts);
  assert(read_csr(medeleg) == exceptions);
}

//
// m_start: machine mode C entry point.
//
void m_start(uintptr_t hartid, uintptr_t dtb) {
  // init the spike file interface (stdin,stdout,stderr)
  // functions with "spike_" prefix are all defined in codes under spike_interface/,
  // sprint is also defined in spike_interface/spike_utils.c
  // 初始化 Spike 文件接口（标准输入、输出和错误输出）
  spike_file_init();
  sprint("In m_start, hartid:%d\n", hartid);

  // init HTIF (Host-Target InterFace) and memory by using the Device Table Blob (DTB)
  // init_dtb() is defined above.
  init_dtb(dtb);

  // set previous privilege mode to S (Supervisor), and will enter S mode after 'mret'
  // write_csr is a macro defined in kernel/riscv.h
  write_csr(mstatus, ((read_csr(mstatus) & ~MSTATUS_MPP_MASK) | MSTATUS_MPP_S));
  //设置先前的特权模式为 S 模式（Supervisor），执行 mret 后将进入 S 模式
  // set M Exception Program Counter to sstart, for mret (requires gcc -mcmodel=medany)
  write_csr(mepc, (uint64)s_start);
// 设置 M 模式异常程序计数器 (mepc) 为 S 模式入口点 `s_start` 的地址
  // delegate all interrupts and exceptions to supervisor mode.
  // delegate_traps() is defined above.
  delegate_traps();

  // switch to supervisor mode (S mode) and jump to s_start(), i.e., set pc to mepc
  asm volatile("mret");
}
