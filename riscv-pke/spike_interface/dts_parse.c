/*
 * Utility functions scanning the Flattened Device Tree (FDT), stored in DTS (Device Tree String).
 *
 * codes are borrowed from riscv-pk (https://github.com/riscv/riscv-pk)
 */

#include "dts_parse.h"
#include "spike_interface/spike_utils.h"
#include "string.h"

// 辅助函数，用于字节序的交换（大端转小端或小端转大端）
static inline uint32 bswap(uint32 x) {
  // 分别处理16位和8位的字节顺序交换
  uint32 y = (x & 0x00FF00FF) << 8 | (x & 0xFF00FF00) >> 8;// 交换字节对
  uint32 z = (y & 0x0000FFFF) << 16 | (y & 0xFFFF0000) >> 16;// 交换16位对
  return z;
}

// 辅助扫描函数，递归处理 FDT 节点和属性
static uint32 *fdt_scan_helper(uint32 *lex, const char *strings, struct fdt_scan_node *node,
                               const struct fdt_cb *cb) {

  // 初始化子节点和属性结构                              
  struct fdt_scan_node child;
  struct fdt_scan_prop prop;
  int last = 0;
  // 是否完成当前节点的标志
  child.parent = node;
  // 设置子节点的父节点和默认地址/大小单元数
  // these are the default cell counts, as per the FDT spec
  child.address_cells = 2;
  child.size_cells = 1;
  prop.node = node;

  while (1) {
    switch (bswap(lex[0])) { // 根据 FDT 类型字段进行判断
      case FDT_NOP: {// 空操作
        lex += 1;// 跳过当前字段
        break;
      }
      case FDT_PROP: {// 节点属性
        assert(!last);// 确保节点未结束
        // 设置属性的名称、长度和值
        prop.name = strings + bswap(lex[2]);
        prop.len = bswap(lex[1]);
        prop.value = lex + 3;
        // 如果属性名是特殊字段，则更新节点信息
        if (node && !strcmp(prop.name, "#address-cells")) {
          node->address_cells = bswap(lex[3]);
        }
        
        if (node && !strcmp(prop.name, "#size-cells")) {
          node->size_cells = bswap(lex[3]);
        }
        lex += 3 + (prop.len + 3) / 4;// 跳过当前属性内容
        cb->prop(&prop, cb->extra);// 调用属性回调
        break;
      }
      case FDT_BEGIN_NODE: {// 开始一个新节点
        uint32 *lex_next;
        if (!last && node && cb->done) cb->done(node, cb->extra);
        // 如果未完成当前节点，调用回调通知完成
        last = 1;// 标记节点已完成
        child.name = (const char *)(lex + 1);// 设置子节点名称
        if (cb->open) cb->open(&child, cb->extra);// 调用节点打开回调

        // 递归扫描子节点
        lex_next = fdt_scan_helper(lex + 2 + strlen(child.name) / 4, strings, &child, cb);
        // 如果关闭回调返回 -1，则删除该节点及其子节点
        if (cb->close && cb->close(&child, cb->extra) == -1)
          while (lex != lex_next) *lex++ = bswap(FDT_NOP);// 用 NOP 占位
        lex = lex_next;
        break;
      }
      case FDT_END_NODE: {// 结束当前节点
        if (!last && node && cb->done) cb->done(node, cb->extra);// 通知节点完成
        return lex + 1;// 返回到父节点
      }
      default: {  // FDT_END
        if (!last && node && cb->done) cb->done(node, cb->extra);
        return lex;
      }
    }
  }
}
// 从节点中提取地址信息
const uint32 *fdt_get_address(const struct fdt_scan_node *node, const uint32 *value,
                              uint64 *result) {
  *result = 0;// 初始化结果为 0
  for (int cells = node->address_cells; cells > 0; --cells)
    *result = (*result << 32) + bswap(*value++);
  return value;// 返回指向下一个值的指针
}
// 从节点中提取大小信息
const uint32 *fdt_get_size(const struct fdt_scan_node *node, const uint32 *value, uint64 *result) {
  *result = 0;
   // 根据节点的大小单元数解析大小值
  for (int cells = node->size_cells; cells > 0; --cells)
    *result = (*result << 32) + bswap(*value++);
  return value;
}
// 主扫描函数，扫描 FDT 的内容
void fdt_scan(uint64 fdt, const struct fdt_cb *cb) {
  struct fdt_header *header = (struct fdt_header *)fdt;

  // Only process FDT that we understand
  if (bswap(header->magic) != FDT_MAGIC || bswap(header->last_comp_version) > FDT_VERSION) return;

  const char *strings = (const char *)(fdt + bswap(header->off_dt_strings));
  uint32 *lex = (uint32 *)(fdt + bswap(header->off_dt_struct));

  fdt_scan_helper(lex, strings, 0, cb);
}
