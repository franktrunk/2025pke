#ifndef _SPIKE_FILE_H_
#define _SPIKE_FILE_H_

#include <unistd.h>// 提供 POSIX 接口，如 read、write 等
#include <sys/stat.h>// 提供文件状态结构和相关函数

#include "util/types.h"
// 定义自定义文件结构体，模拟文件管理
typedef struct file {
  int kfd;  // file descriptor of the host file
  uint32 refcnt; // 引用计数，记录该文件的引用次数
} spike_file_t;

extern spike_file_t spike_files[];

#define O_RDONLY 00 // 只读模式
#define O_WRONLY 01  // 只写模式
#define O_RDWR 02 // 读写模式
#define ENOMEM 12 /* Out of memory */// 内存不足错误码

#define stdin (spike_files + 0)// 标准输入对应的文件
#define stdout (spike_files + 1)// 标准输出对应的文件
#define stderr (spike_files + 2)// 标准错误对应的文件

#define INIT_FILE_REF 3

struct frontend_stat {
  uint64 dev;// 设备号
  uint64 ino;// 文件节点号
  uint32 mode;// 文件权限和类型
  uint32 nlink;// 硬链接数
  uint32 uid;// 文件所有者的用户 ID
  uint32 gid;// 文件所属组的组 ID
  uint64 rdev;// 设备文件号（如果是设备文件）
  uint64 __pad1;// 保留字段
  uint64 size;// 文件大小（以字节为单位）
  uint32 blksize;// 文件系统的块大小
  uint32 __pad2; // 保留字段
  uint64 blocks;// 文件占用的块数
  uint64 atime;// 文件最近访问时间
  uint64 __pad3;// 保留字段
  uint64 mtime;// 文件最近修改时间
  uint64 __pad4;// 保留字段
  uint64 ctime;// 文件状态最近改变时间
  uint64 __pad5;// 保留字段
  uint32 __unused4;// 未使用字段
  uint32 __unused5;// 未使用字段
};
// 将自定义文件状态结构体的内容复制到标准文件状态结构体
void copy_stat(struct stat* dest, struct frontend_stat* src);
// 打开文件，返回自定义文件结构指针
spike_file_t* spike_file_open(const char* fn, int flags, int mode);
// 关闭文件
int spike_file_close(spike_file_t* f);
// 在指定目录下打开文件
spike_file_t* spike_file_openat(int dirfd, const char* fn, int flags, int mode);
ssize_t spike_file_lseek(spike_file_t* f, size_t ptr, int dir);// 文件指针偏移操作
ssize_t spike_file_read(spike_file_t* f, void* buf, size_t size);// 读取文件内容
ssize_t spike_file_pread(spike_file_t* f, void* buf, size_t n, off_t off);// 从指定偏移位置读取文件内容
ssize_t spike_file_write(spike_file_t* f, const void* buf, size_t n);// 写入文件内容
void spike_file_decref(spike_file_t* f);// 减少文件的引用计数，必要时释放资源
void spike_file_init(void);// 初始化文件管理系统
int spike_file_dup(spike_file_t* f);// 复制文件描述符
int spike_file_truncate(spike_file_t* f, off_t len);// 截断文件到指定长度
int spike_file_stat(spike_file_t* f, struct stat* s);// 获取文件状态

#endif// 结束宏定义 _SPIKE_FILE_H_
