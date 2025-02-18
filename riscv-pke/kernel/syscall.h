/*
 * define the syscall numbers of PKE OS kernel.
 */
#ifndef _SYSCALL_H_
#define _SYSCALL_H_

// syscalls of PKE OS kernel. append below if adding new syscalls.
// PKE 操作系统内核的系统调用编号。
// 如果需要添加新的系统调用，请在下面追加新的编号。
#define SYS_user_base 64
// 用户程序可以调用的具体系统调用编号。编号由基数 SYS_user_base 加上偏移量。
#define SYS_user_print (SYS_user_base + 0)
#define SYS_user_exit (SYS_user_base + 1)
// do_syscall 函数的声明。该函数用于处理内核中的系统调用。
// a0 到 a7 是传递给系统调用的参数，返回值由函数返回。
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7);

#endif
