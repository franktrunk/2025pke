/*
 * accessing host files by using the Spike interface.
 *
 * PKE OS needs to access the host file duing its execution to conduct ELF (application) loading.
 *
 * codes are borrowed from riscv-pk (https://github.com/riscv/riscv-pk)
 */

#include "spike_file.h"// 文件接口的定义
#include "spike_htif.h"// HTIF (Host-Target Interface) 定义
#include "atomic.h"// 原子操作相关的定义
#include "string.h"
#include "util/functions.h"
#include "spike_interface/spike_utils.h"
//#include "../kernel/config.h"
// 文件和文件描述符的最大数量
#define MAX_FILES 128
#define MAX_FDS 128
// 全局变量：文件描述符数组
static spike_file_t* spike_fds[MAX_FDS];
spike_file_t spike_files[MAX_FILES] = {[0 ... MAX_FILES - 1] = {-1, 0}};
// 将frontend_stat结构体的数据复制到标准的stat结构体中
void copy_stat(struct stat* dest_va, struct frontend_stat* src) {
  struct stat* dest = (struct stat*)dest_va;
  dest->st_dev = src->dev;
  dest->st_ino = src->ino;
  dest->st_mode = src->mode;
  dest->st_nlink = src->nlink;
  dest->st_uid = src->uid;
  dest->st_gid = src->gid;
  dest->st_rdev = src->rdev;
  dest->st_size = src->size;
  dest->st_blksize = src->blksize;
  dest->st_blocks = src->blocks;
  dest->st_atime = src->atime;
  dest->st_mtime = src->mtime;
  dest->st_ctime = src->ctime;
}
// 获取文件的状态信息，并将其复制到目标结构体中
int spike_file_stat(spike_file_t* f, struct stat* s) {
  struct frontend_stat buf;
  uint64 pa = (uint64)&buf;
  long ret = frontend_syscall(HTIFSYS_fstat, f->kfd, (uint64)&buf, 0, 0, 0, 0, 0);
  copy_stat(s, &buf);
  return ret;
}
// 关闭文件描述符
int spike_file_close(spike_file_t* f) {
  if (!f) return -1;
  // 使用原子操作更新文件描述符数组
  spike_file_t* old = atomic_cas(&spike_fds[f->kfd], f, 0);
  spike_file_decref(f);
  if (old != f) return -1;
  spike_file_decref(f);
  return 0;
}
// 递减文件引用计数
void spike_file_decref(spike_file_t* f) {
   // 如果引用计数为2，说明此文件描述符只剩一个引用，准备关闭文件
  if (atomic_add(&f->refcnt, -1) == 2) {
    int kfd = f->kfd;
    mb();
    atomic_set(&f->refcnt, 0);

    frontend_syscall(HTIFSYS_close, kfd, 0, 0, 0, 0, 0, 0);
  }
}
// 增加文件的引用计数
void spike_file_incref(spike_file_t* f) {
  // 调用前端系统调用写入数据
  long prev = atomic_add(&f->refcnt, 1);
  kassert(prev > 0);
}

ssize_t spike_file_write(spike_file_t* f, const void* buf, size_t size) {
  return frontend_syscall(HTIFSYS_write, f->kfd, (uint64)buf, size, 0, 0, 0, 0);
}
// 调用前端系统调用写入数据
static spike_file_t* spike_file_get_free(void) {
  for (spike_file_t* f = spike_files; f < spike_files + MAX_FILES; f++)
    if (atomic_read(&f->refcnt) == 0 && atomic_cas(&f->refcnt, 0, INIT_FILE_REF) == 0)
      return f;
  return NULL;
}
// 复制文件描述符，增加其引用计数
int spike_file_dup(spike_file_t* f) {
  for (int i = 0; i < MAX_FDS; i++) {
    if (atomic_cas(&spike_fds[i], 0, f) == 0) {
      spike_file_incref(f);
      return i;
    }
  }
  return -1;
}

void spike_file_init(void) {
  // create stdin, stdout, stderr and FDs 0-2
  for (int i = 0; i < 3; i++) {
    spike_file_t* f = spike_file_get_free();
    f->kfd = i;
    spike_file_dup(f);
  }
}
// 打开指定路径的文件（使用目录文件描述符）
spike_file_t* spike_file_openat(int dirfd, const char* fn, int flags, int mode) {
  spike_file_t* f = spike_file_get_free();
  if (f == NULL) return ERR_PTR(-ENOMEM);

  size_t fn_size = strlen(fn) + 1;
  long ret = frontend_syscall(HTIFSYS_openat, dirfd, (uint64)fn, fn_size, flags, mode, 0, 0);
  if (ret >= 0) {
    f->kfd = ret;
    return f;
  } else {
    spike_file_decref(f);
    return ERR_PTR(ret);
  }
}
// 打开指定路径的文件（默认使用当前工作目录）
spike_file_t* spike_file_open(const char* fn, int flags, int mode) {
  return spike_file_openat(AT_FDCWD, fn, flags, mode);
}
// 从文件中读取数据，支持指定偏移量
ssize_t spike_file_pread(spike_file_t* f, void* buf, size_t size, off_t offset) {
  return frontend_syscall(HTIFSYS_pread, f->kfd, (uint64)buf, size, offset, 0, 0, 0);
}
// 从文件中读取数据
ssize_t spike_file_read(spike_file_t* f, void* buf, size_t size) {
  return frontend_syscall(HTIFSYS_read, f->kfd, (uint64)buf, size, 0, 0, 0, 0);
}
// 从文件中读取数据
ssize_t spike_file_lseek(spike_file_t* f, size_t ptr, int dir) {
  return frontend_syscall(HTIFSYS_lseek, f->kfd, ptr, dir, 0, 0, 0, 0);
}
