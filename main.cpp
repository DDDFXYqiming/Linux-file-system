#include"simdisk.h"
#include"function.h"
#include"order.h"
#include<iostream>
#include<string>
#include<fstream>
#include <windows.h>
using namespace std;

int iNode_bitmap[INODE_NUMBER];  //i结点位图
i_Node iNode_table[INODE_NUMBER];  //i结点表
int blocks_bitmap[BlOCKS];       //盘块位图
BlockGroup blockGroups[BLOCK_GROUP_NUMBER];  //数据块表


Dir curDir;   //当前目录
int curUser = -1;  //当前用户
fstream f_simdisk;    //虚拟磁盘文件
User users[MAX_USER];  //用户列表


int BUF_SIZE = 4096;


shareData sd;
int main()
{	
	//cout << DATA_ADDR << endl;
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
	LPVOID lpBase;
	if (hMapFile == NULL)
	{// 创建共享文件句柄 
		HANDLE hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,   // 物理文件句柄
			NULL,   // 默认安全级别
			PAGE_READWRITE,   // 可读可写
			0,   // 高位文件大小
			BUF_SIZE,   // 低位文件大小
			"ShareMemorySZHC"   // 共享内存名称
		);

		// 映射缓存区视图 , 得到指向共享内存的指针
			lpBase = MapViewOfFile(
			hMapFile,            // 共享内存的句柄
			FILE_MAP_ALL_ACCESS, // 可读写许可
			0,
			0,
			BUF_SIZE
		);
	}
	else
	{
		lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	}
	f_simdisk.open("simdisk", ios::in | ios::binary);
	if (!f_simdisk.is_open())
	{
		cout << "无模拟文件系统存在，是否初始化 Y/N\n";
		char a;
		cin >> a;
		if (a == 'Y' || a == 'y')
		{
			disk_init();	
		}
		else
		{
			return 0;
		}
		
	}
	f_simdisk.close();
	read_info();  //读取磁盘信息
	int m;  
	cout << "选择模式（0后台模式，1命令行模式）" << endl;
	cin >> m;  //0后台模式，1命令行模式
	sd.uNumber = -1;
	share_write(sd, lpBase);
	if (m == 0)
	{//后台模式

		sd.flag = 0;
		share_write(sd, lpBase);
		int i;
		while (1)
		{
			//登录过程
			Sleep(100);
			share_read(sd, lpBase);
			if (sd.flag == 1 && curUser == -1)
			{
				for (i = 0; i < MAX_USER; i++)
				{
					if ((sd.data1 == users[i].uName) && (sd.data2 == users[i].password))
					{
						curUser = i;
						break;//登录成功
					}
				}
				if (i == MAX_USER)
				{
					cout << "用户名或密码错误" << endl;
					show_shell("用户名或密码错误", lpBase);
					sd.flag = 0;
					share_write(sd, lpBase);
				}
			}
			else if (curUser >= 0)
			{
				break;
			}
		}
		//cout << 1 << endl;
		while (1)
		{
			while (1)  //等待shell输出结果
			{
				share_read(sd, lpBase);
				if (sd.data3 == "")  //等待shell输出完成
				{
					break;
				}
			}
			sd.data3 = get_path(curDir) + "/>"; //当前路径
			sd.flag = 2;  //设置flag让shell输出
			share_write(sd, lpBase);  //在shell输出路径
			while (1)//等待flag = 1
			{
				share_read(sd, lpBase);  //循环读取共享内存
				if (sd.flag == 1)
				{
					break;
				}
			}
			if (sd.flag == 1) //处理数据
			{
				process_shell(sd);
			}
			
		}
		// 解除文件映射
		UnmapViewOfFile(lpBase);
		// 关闭内存映射文件对象句柄
		CloseHandle(hMapFile);
	}
	else
	{//命令行模式
		string order;
		while (curUser == -1)
		{
			int b = login();
			if (b == -1)
			{
				cout << "用户不存在或密码错误" << endl;
			}
			else
			{
				break;
			}
		}
		while (1)
		{
			cout << get_path(curDir) << "/>";
			cin >> order;
			if (order == "EXIT" || order == "exit")
			{
				break;
			}
			process(order);
		}
	}
	/*f_simdisk.open("simdisk", ios::in | ios::binary);
	f_simdisk.seekg(iNode_table[curDir.iNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));
	f_simdisk.close();
	cout << curDir.iNode << endl;
	cout << iNode_table[curDir.iNode].BlockAddr << endl;
	cout << iNode_table[curDir.iNode].Name << endl;*/

	/*for (int i = 0; i < 10; i++)
	{
		cout << iNode_bitmap[i] << endl;
	}*/





	//system("pause");
}

void show_shell(string s, LPVOID lpBase)
{
	sd.flag = 3;
	sd.data3 = s;
	memcpy(lpBase, &sd, sizeof(shareData));
}
void process_shell(shareData sd)
{
	if (sd.order == "cd")
	{
		string pathName;
		order_cd(sd.data1);
	}
	else if (sd.order == "md")
	{
		string pathName;
		order_md(sd.data1, curDir);
	}
	else if (sd.order == "rd")
	{
		string pathName;
		order_rd(sd.data1);
		read_info();

	}
	else if (sd.order == "newfile")
	{
		string filename;
		order_newfile(sd.data1, sd.data2);

	}
	else if (sd.order == "cat")
	{
		string pathName;

		order_cat(sd.data1);
	}
	else if (sd.order == "del")
	{
		string pathName;
		order_rd(sd.data1);
		read_info();

	}
	else if (sd.order == "copy")
	{
		string path1, path2, str1, str2;
		path1 = sd.data1;
		path2 = sd.data2;
		str1 = path1.substr(0, 6);
		str2 = path2.substr(0, 6);
		if (str1 != "<host>" && str2 != "<host>")
		{
			order_copy(path1, path2);
		}
		else
		{
			if (str1 == "<host>")
			{
				order_copy_host_to_sim(path1, path2);
			}
			else if (str2 == "<host>")
			{
				order_copy_sim_to_host(path1, path2);
			}

		}
		read_info();
	}
	else
	{
		cout << "无此命令" << endl;
	}
}