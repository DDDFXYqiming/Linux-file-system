#pragma once

#include <fstream>
#include <string>

// 磁盘布局：与课程设计一致
// 元数据区（用户表 + 块组 + i节点位图 + i节点表 + 磁盘块位图）+ 数据区（100MB）

constexpr int BLOCK_SIZE = 1024;              // 磁盘块大小 1KB
constexpr int BLOCK_NUMBER = 1024;            // 每块组磁盘块数
constexpr int BLOCK_GROUP_NUMBER = 100;       // 块组数量
constexpr int BLOCKS = BLOCK_NUMBER * BLOCK_GROUP_NUMBER;  // 磁盘块总数 102400
constexpr int INODE_NUMBER = BLOCK_NUMBER * BLOCK_GROUP_NUMBER; // i节点总数

constexpr int MAX_SUBFILE = 300;              // 目录最大子项数
constexpr int MAX_USER = 10;                  // 最大用户数

// 位图状态
constexpr int USED = 1;
constexpr int NOT_USED = 0;

// 文件类型
constexpr int DIRECTORY_TYPE = 0;
constexpr int FILE_TYPE = 1;

// 用户类型
constexpr int ROOT_USER = 3;
constexpr int SYSTEM_USER = 2;
constexpr int USER = 1;

// 文件保护码
constexpr int SHARED = 0;
constexpr int READ_ONLY = 1;
constexpr int READ_WRITE = 2;
constexpr int SYSTEM = 3;

// 用户
struct User {
    std::string uName;    // 用户名
    std::string password; // 密码
    int uType;            // 用户类型
};

// 目录项
struct Dir_ {
    int iNode;            // 目录自身 i节点号
    int iNode_number;     // 子项数量
    int iNodes[MAX_SUBFILE]; // 子项 i节点号
};
using Dir = Dir_;

constexpr int DIR_SIZE = sizeof(Dir) / BLOCK_SIZE + 1; // 一个目录占用块数

// 超级块
struct SuperBlock {
    int all_freeBlocks;   // 空闲磁盘块数
    int all_freeiNodes;   // 空闲 i节点数
};

// 数据块组
struct BlockGroup {
    SuperBlock sBlock;
    int BlockMap;         // 块位图偏移
    int BlockAddr;        // 数据起始地址
    int iNodeMap;         // i节点位图偏移
    int iNodeTable;       // i节点表偏移
    int freeiNode;
    int freeBlocks;
};

// i节点
struct i_Node {
    std::string Name;     // 文件名
    int uNumber;          // 属主用户序号
    int fileType;         // 文件类型
    int size;             // 文件大小
    int BlockNumber;      // 占用块数
    int BlockAddr;        // 数据起始地址
    int father_iNode;     // 父目录 i节点号
};

// 共享内存通信协议（与 shell 进程交互）
struct shareData {
    int flag;        // 0=空闲 1=命令/登录 2=路径提示 3=输出
    int uNumber;     // 写者用户序号，-1 表示无写者
    std::string order;  // 命令名
    std::string data1;  // 参数1
    std::string data2;  // 参数2
    std::string data3;  // 输出/路径
};

// 数据区起始地址（= 元数据总大小）
constexpr long long DATA_ADDR =
    static_cast<long long>(sizeof(BlockGroup)) * BLOCK_GROUP_NUMBER +
    static_cast<long long>(sizeof(int)) * (INODE_NUMBER + BLOCKS) +
    static_cast<long long>(sizeof(i_Node)) * INODE_NUMBER +
    static_cast<long long>(sizeof(User)) * MAX_USER;

// 全局状态（与原版布局一致）
extern int iNode_bitmap[INODE_NUMBER];
extern i_Node iNode_table[INODE_NUMBER];
extern int blocks_bitmap[BLOCKS];
extern BlockGroup blockGroups[BLOCK_GROUP_NUMBER];
extern Dir curDir;
extern int curUser;
extern std::fstream f_simdisk;
extern User users[MAX_USER];
