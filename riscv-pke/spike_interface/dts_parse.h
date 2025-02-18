#ifndef _DT_PARSE_H_
#define _DT_PARSE_H_

#include "util/types.h"
// FDT（Flattened Device Tree）的magic数和版本号
#define FDT_MAGIC 0xd00dfeed
#define FDT_VERSION 17

struct fdt_header {
  uint32 magic;
  uint32 totalsize;// FDT的总大小（字节数）
  uint32 off_dt_struct;// 设备树结构的偏移量
  uint32 off_dt_strings; // 字符串部分的偏移量
  uint32 off_mem_rsvmap;// 内存预留区域的偏移量
  uint32 version;// 设备树版本
  uint32 last_comp_version;  // 最后兼容的版本（<= 17）
  uint32 boot_cpuid_phys;// 启动时使用的物理CPU ID
  uint32 size_dt_strings; // 设备树字符串部分的大小
  uint32 size_dt_struct;// 设备树结构部分的大小
};
// FDT标记类型，用于识别设备树中的节点和属性类型
#define FDT_BEGIN_NODE 1// 开始一个新节点
#define FDT_END_NODE 2 // 结束一个节点
#define FDT_PROP 3// 节点的属性
#define FDT_NOP 4// 空操作（占位符）
#define FDT_END 9  // 设备树的结束标志

struct fdt_scan_node {
  const struct fdt_scan_node *parent;// 当前节点的父节点
  const char *name;// 节点的名称
  int address_cells;// 地址单元数
  int size_cells; // 大小单元数
};

struct fdt_scan_prop {
  const struct fdt_scan_node *node; // 属性所属的节点
  const char *name;  // 属性的名称
  uint32 *value;  // 属性值的指针
  int len;  // in bytes of value
};

struct fdt_cb {
  void (*open)(const struct fdt_scan_node *node, void *extra);// 节点打开时的回调
  void (*prop)(const struct fdt_scan_prop *prop, void *extra);// 遇到属性时的回调
  void (*done)(const struct fdt_scan_node *node,// 扫描完节点的属性时的回调
               void *extra);  // last property was seen
  int (*close)(const struct fdt_scan_node *node,// 关闭节点时的回调，返回-1表示删除该节点及其子节点
               void *extra);  // -1 => delete the node + children
  void *extra;// 传递给回调函数的额外数据
};

// Scan the contents of FDT
// 扫描FDT的内容
void fdt_scan(uint64 fdt, const struct fdt_cb *cb);
// 获取FDT的大小
uint32 fdt_size(uint64 fdt);

// Extract fields
// 从节点中提取地址
const uint32 *fdt_get_address(const struct fdt_scan_node *node, const uint32 *base, uint64 *value);
// 从节点中提取大小
const uint32 *fdt_get_size(const struct fdt_scan_node *node, const uint32 *base, uint64 *value);
// 在属性的字符串列表中查找指定字符串的索引
int fdt_string_list_index(const struct fdt_scan_prop *prop,
                          const char *str);  // -1 if not found
#endif
