#ifndef _ELF_H_
#define _ELF_H_

#include "util/types.h"
#include "process.h"

#define MAX_CMDLINE_ARGS 64// 支持的最大命令行参数数量。
/*
 * 该头文件定义了用于解析和加载 ELF（Executable and Linkable Format，可执行与可链接格式）文件的数据结构和函数。
 * 它支持处理 ELF 文件头、程序段头，并提供初始化和加载 ELF 文件的工具函数。
 */

// elf header structure
// 定义 ELF 文件头结构，包含 ELF 文件的元数据信息。
typedef struct elf_header_t {
  uint32 magic;
  uint8 elf[12];   // ELF 格式的标识字节。
  uint16 type;      /* Object file type */// 对象文件类型（例如，可重定位文件、可执行文件、共享文件）。
  uint16 machine;   /* Architecture */// 目标架构。
  uint32 version;   /* Object file version */// 对象文件版本。
  uint64 entry;     /* Entry point virtual address */// 入口点虚拟地址。
  uint64 phoff;     /* Program header table file offset */// 程序头表在文件中的偏移。
  uint64 shoff;     /* Section header table file offset */// 节头表在文件中的偏移。
  uint32 flags;     /* Processor-specific flags */ // 特定处理器标志。
  uint16 ehsize;    /* ELF header size in bytes */ // ELF 文件头大小（字节）。
  uint16 phentsize; /* Program header table entry size */ // 程序头表项大小。
  uint16 phnum;     /* Program header table entry count */ // 程序头表项数量。
  uint16 shentsize; /* Section header table entry size */ // 节头表项大小。
  uint16 shnum;     /* Section header table entry count */  // 节头表项数量。
  uint16 shstrndx;  /* Section header string table index */  // 节头字符串表索引。
} elf_header;

// Program segment header.
// 定义程序段头结构。
typedef struct elf_prog_header_t {
  uint32 type;   /* Segment type */ // 段类型（如可加载段）。
  uint32 flags;  /* Segment flags */  // 段标志（如可读、可写、可执行）。
  uint64 off;    /* Segment file offset */ // 段在文件中的偏移。
  uint64 vaddr;  /* Segment virtual address */ // 段的虚拟地址。
  uint64 paddr;  /* Segment physical address */ // 段的物理地址。
  uint64 filesz; /* Segment size in file */ // 段在文件中的大小。
  uint64 memsz;  /* Segment size in memory */ // 段在内存中的大小
  uint64 align;  /* Segment alignment */ // 段的对齐要求。
} elf_prog_header;

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian ELF 文件的魔数
#define ELF_PROG_LOAD 1  // 可加载段的类型标志。

// 定义 ELF 文件加载状态的枚举。
typedef enum elf_status_t {
  EL_OK = 0,  // 成功。
  EL_EIO,     // 输入输出错误。
  EL_ENOMEM,  // 内存不足。
  EL_NOTELF,  // 非 ELF 文件。
  EL_ERR,     // 通用错误。

} elf_status;

// 定义 ELF 上下文结构，用于跟踪 ELF 文件的加载过程。
typedef struct elf_ctx_t {
  void *info;
  elf_header ehdr;
} elf_ctx;

// 初始化 ELF 上下文，读取 ELF 文件头。
elf_status elf_init(elf_ctx *ctx, void *info);
// 加载 ELF 文件的段到内存中。
elf_status elf_load(elf_ctx *ctx);
// 从主机 ELF 文件中加载用户程序的二进制代码。
void load_bincode_from_host_elf(process *p);

#endif
