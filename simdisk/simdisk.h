#pragma once
#include<windows.h>
#include<string>
using namespace std;

#define BLOCK_SIZE          1024  //盘块大小为1KB
#define BLOCK_NUMBER        1024  //1024个盘块组成数据块
#define BLOCK_GROUP_NUMBER  100   //100个数据块组成磁盘
#define BlOCKS              (BLOCK_NUMBER * BLOCK_GROUP_NUMBER)  //盘块总数

#define INODE_NUMBER        (BLOCK_NUMBER * BLOCK_GROUP_NUMBER)  //每个盘块都有i结点

#define MAX_SUBFILE         300   //目录下最大子文件数目

#define DIR_SIZE            (sizeof(Dir)/BLOCK_SIZE + 1)  //一个目录需要的盘块数

//位图
#define USED                1   
#define NOT_USED            0


//i结点类型
#define DIRECTORY_TYPE      0            //目录
#define FILE_TYPE           1            //文件

//用户类型
#define ROOT_USER           3         
#define SYSTEM_USER         2
#define USER                1


//文件属性
#define SHARED              0
#define READ_ONLY           1
#define READ_WRITE          2
#define SYSTEM              3

//最大用户数量
#define MAX_USER            10

//初始地址
#define DATA_ADDR           (sizeof(BlockGroup) * BLOCK_GROUP_NUMBER + sizeof(int) * (INODE_NUMBER + BlOCKS) + sizeof(i_Node) * INODE_NUMBER + sizeof(User) * MAX_USER)

//用户类
class User      
{
public:
	string uName;  //用户名
	string password; //密码
	int uType;   //用户类型
};

//目录类
class Dir        
{
public:
	int iNode;      //i结点表索引
	int iNode_number; //子文件i结点数量
	int iNodes[MAX_SUBFILE];      //子文件i结点数组 

};

//超级块
class SuperBlock
{
public:
	int all_freeBlocks;   //空闲盘块总数
	int all_freeiNodes;   //空闲i结点总数

};

//数据块
class BlockGroup
{
public:
	SuperBlock sBlock;  //超级块
	//数据块信息
	int BlockMap;    //数据块开始位置的位图索引值
	int BlockAddr;   //首地址	
	int iNodeMap;    //i结点位图索引
	int iNodeTable;  //i结点表索引
	int freeiNode;   //空闲i结点
	int freeBlocks;  //空闲盘块总数

};

//i-结点
class i_Node
{
public:
	string Name;   //文件名
	int uNumber;    //文件所有者索引
	int fileType;    //文件类型
	int size;        //文件大小
	int BlockNumber; //占用盘块数
	int BlockAddr;   //起始地址
	int father_iNode;//上一级目录i结点索引
};

struct shareData
{
public:
	int flag;
	int uNumber;
	string order;
	string data1; //参数1
	string data2; //参数2
	string data3; //结果
};


extern int iNode_bitmap[INODE_NUMBER];              //i结点位图
extern i_Node iNode_table[INODE_NUMBER];            //i结点表
extern int blocks_bitmap[BlOCKS];                   //盘块位图
extern BlockGroup blockGroups[BLOCK_GROUP_NUMBER];  //数据块表


extern Dir curDir;   //当前目录
extern int curUser;  //当前用户索引
extern fstream f_simdisk;    //虚拟磁盘文件

extern User users[MAX_USER];



extern shareData sd;
