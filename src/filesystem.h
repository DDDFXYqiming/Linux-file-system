#pragma once

#include "simdisk.h"
#include <string>

// 磁盘核心操作
void disk_init();
void read_info();
void save_all();

// 根据目录得到完整路径
std::string get_path(const Dir& dir);
// 返回可用 i节点号，无则 -1
int check_iNode();
// 返回连续 size 块的起始块号，无则 -1
int check_blocks(int size);
// 按路径查找文件/目录，返回 i节点号，未找到返回 -1
int find_by_pathName(const std::string& pathName);
// 数据地址 → 磁盘块号
int addr_to_bitmap(long long addr);
// 递归删除 i节点（文件或目录）
void delete_file(int iNode);
// 宿主路径转换（<host>D:\data\sample\test.txt → D:\data\sample\test.txt）
std::string host_path(const std::string& pathName);
// 从宿主路径提取文件名
std::string host_filename(const std::string& pathName);
