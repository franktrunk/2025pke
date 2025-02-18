// See LICENSE for license details.
// borrowed from https://github.com/riscv/riscv-pk:
// machine/atomic.h

/* 自旋锁（Spinlock） 是一种用于实现线程或进程同步的锁，它的工作原理是当一个线程尝试获
** 取锁时，如果锁已经被其他线程持有，线程不会进入休眠或等待队列，而是持续循环检查锁的
   状态，直到锁被释放。这种检查过程称为“自旋”，因此得名“自旋锁”。
*/

#ifndef _RISCV_ATOMIC_H_
#define _RISCV_ATOMIC_H_

// Currently, interrupts are always disabled in M-mode.
// todo: for PKE, wo turn on irq in lab_1_3_timer, so wo have to implement these two functions.
#define disable_irqsave() (0)// 禁用中断并保存当前状态（这里返回0作为占位符）
#define enable_irqrestore(flags) ((void)(flags))  // 恢复中断状态（这里不做实际操作）

typedef struct {
  int lock;  // 锁的状态：0表示锁未被占用，非0表示锁被占用
  // For debugging:
  char* name;       // Name of lock.
  struct cpu* cpu;  // The cpu holding the lock. 持有该锁的CPU
} spinlock_t;
// 初始化自旋锁：默认值为0，表示未被占用
#define SPINLOCK_INIT \
  { 0 }
// 内存屏障：确保指令顺序，防止编译器和硬件重排
#define mb() asm volatile("fence" ::: "memory")
// 原子操作：将指定的值设置为ptr指向的内存位置的值
#define atomic_set(ptr, val) (*(volatile typeof(*(ptr))*)(ptr) = val)
// 原子操作：读取ptr指向内存位置的值
#define atomic_read(ptr) (*(volatile typeof(*(ptr))*)(ptr))
// 原子二元操作：执行操作并返回原始值

#define atomic_binop(ptr, inc, op)         \
  ({                                       \
    long flags = disable_irqsave();        \
    typeof(*(ptr)) res = atomic_read(ptr); \
    atomic_set(ptr, op);                   \
    enable_irqrestore(flags);              \
    res;                                   \
  })
  // 原子加法：执行加法操作并返回原始值
#define atomic_add(ptr, inc) atomic_binop(ptr, inc, res + (inc))
// 原子或操作：执行按位或操作并返回原始值
#define atomic_or(ptr, inc) atomic_binop(ptr, inc, res | (inc))
// 原子交换操作：执行交换操作并返回原始值
#define atomic_swap(ptr, inc) atomic_binop(ptr, inc, (inc))
// 原子比较交换：如果ptr指向的值等于cmp，则将其设置为swp并返回原始值
#define atomic_cas(ptr, cmp, swp)                           \
  ({                                                        \
    long flags = disable_irqsave();                         \
    typeof(*(ptr)) res = *(volatile typeof(*(ptr))*)(ptr);  \
    if (res == (cmp)) *(volatile typeof(ptr))(ptr) = (swp); \
    enable_irqrestore(flags);                               \
    res;                                                    \
  })
// 尝试获取自旋锁，成功返回0，失败返回锁的值
static inline int spinlock_trylock(spinlock_t* lock) {
  int res = atomic_swap(&lock->lock, -1); // 使用原子交换操作尝试获取锁
  mb();                      // 执行内存屏障以确保操作顺序
  return res;
}
// 获取自旋锁：如果锁被占用，则一直循环尝试获取，直到成功
static inline void spinlock_lock(spinlock_t* lock) {
  do {
    while (atomic_read(&lock->lock))// 如果锁已被占用，等待
      ;
  } while (spinlock_trylock(lock));// 尝试获取锁，直到成功
}
// 释放自旋锁
static inline void spinlock_unlock(spinlock_t* lock) {
  mb();// 执行内存屏障
  atomic_set(&lock->lock, 0);// 将锁状态设置为0，表示释放锁
}

// 获取自旋锁并禁用中断：返回当前中断状态，之后可以通过unlock_irqrestore恢复
static inline long spinlock_lock_irqsave(spinlock_t* lock) {
  long flags = disable_irqsave(); // 禁用中断并保存当前中断状态
  spinlock_lock(lock);// 获取自旋锁
  return flags;// 返回保存的中断状态
}

// 释放自旋锁并恢复中断状态
static inline void spinlock_unlock_irqrestore(spinlock_t* lock, long flags) {
  spinlock_unlock(lock);// 释放自旋锁
  enable_irqrestore(flags);// 恢复中断状态
}

#endif
/*带有中断保护的自旋锁（spinlock），用于确保在锁定期间中断被禁用，从而避免中断处理程序
在临界区内被触发，保证在临界区内的操作不会被中断打断。*/