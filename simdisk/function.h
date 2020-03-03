#pragma once
#include<windows.h>
void disk_init();       //初始化
void read_info();       //读取磁盘信息
string get_path(Dir);   //返回目录的完整路径
int check_iNode();      //返回可用的i-结点号
int check_blocks(int size);   //返回可用的盘块号
int find_by_pathName(string pathName);  //根据路径，返回最终文件或目录的i结点号
int addr_to_bitmap(int);   //地址到盘块号的转换
void delete_file(int);     //删除文件
string host_path(string pathName);    //宿主机路径转换
string host_filename(string pathName);  //根据路径获取宿主机文件名
int login();   //登录
void add_user(string uName, string password, string uType);   //添加用户
void delete_user(string uName);   //删除用户

void share_read(shareData&, LPVOID);
void share_write(shareData&, LPVOID);
void process_shell(shareData sd);
void show_shell(string, LPVOID);