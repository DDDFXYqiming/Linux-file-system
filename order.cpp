#include"simdisk.h"
#include"order.h"
#include"function.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<sstream>
using namespace std;

void order_info()      //info命令
{
	cout << " 系统信息\n";
	cout << setw(15) << "  磁盘总空间： " << 100 * 1024 * 1024 << "B" << endl;
	cout << setw(15) << "  已用空间： " << 100 * 1024 * 1024 - blockGroups[0].sBlock.all_freeBlocks * 1024 << " B" << endl;
	cout << setw(15) << "  剩余空间： " << blockGroups[0].sBlock.all_freeBlocks * 1024 << " B" << endl;

	cout << "\n 数据块信息\n";
	cout << setw(15) << "  数据块组数： " << "100" << endl;
	cout << setw(15) << "  每组盘块数： " << "1024" << endl;

	cout << "\n 盘块信息\n";
	cout << setw(15) << "  盘块大小： " << "1024 B" << endl;
	cout << setw(15) << "  盘块数量： " << "102400" << endl;
	cout << setw(15) << "  已用盘块： " << 102400 - blockGroups[0].sBlock.all_freeBlocks << endl;
	cout << setw(15) << "  剩余盘块数： " << blockGroups[0].sBlock.all_freeBlocks << endl;
}

void order_cd(string pathName)
{

	fstream f_simdisk;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	if (!f_simdisk) {
		cout << "error_order_cd" << endl;
	}

	int pos = 0, n = curDir.iNode_number, diriNode; 
	Dir tempDir = curDir;
	string str;


	if (pathName == "..") //上一级目录
	{
		diriNode = iNode_table[curDir.iNode].father_iNode; //获取上一级目录i-结点
		f_simdisk.seekg(iNode_table[diriNode].BlockAddr, ios::beg);
		f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));
		f_simdisk.close();
		return;
	}
	if (pathName == "/")  //根目录
	{
		f_simdisk.seekg(iNode_table[0].BlockAddr, ios::beg);
		f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));
		f_simdisk.close();
		return;
	}
	//循环查找路径
	while (pathName.find('/', pos) != pathName.npos)
	{
		str = pathName.substr(pos, pathName.find('/', pos) - pos);
		n = tempDir.iNode_number;
		while (n)
		{
			if (iNode_table[tempDir.iNodes[n - 1]].Name == str)
			{
				diriNode = tempDir.iNodes[n - 1];
				break;
			}
			n--;
		}
		if (n == 0)
		{
			HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
			LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			cout << "不存在目录或文件" << str << endl;
			show_shell("目录不存在\n", lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return;
		}
		f_simdisk.seekg(iNode_table[diriNode].BlockAddr, ios::beg);
		f_simdisk.read(reinterpret_cast<char *>(&tempDir), sizeof(Dir));
		pos = pathName.find('/', pos) + 1;
	}//cout << 1 << endl;
	
	str = pathName.substr(pos);
	n = tempDir.iNode_number;
	while (n)
	{
		if (iNode_table[tempDir.iNodes[n - 1]].Name == str)
		{
			diriNode = tempDir.iNodes[n - 1];
			break;
		}
		n--;
	}//cout << 2 << endl;
	
	if(n == 0)
	{
		cout << "不存在目录或文件" << str << endl;
		HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
		LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		show_shell("目录不存在\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}

	//找到要进入目录的i-结点后，读取磁盘，赋值到curDir
	f_simdisk.seekg(iNode_table[diriNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));

	f_simdisk.close();
}
void order_md(string dirName, Dir &mdDir)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	string shell;
	int i, newiNode, newBlocks, j;
	//判断空间是否足够
	if (mdDir.iNode_number == MAX_SUBFILE)
	{
		cout << "当前目录无法创建更多的文件\n";  //原来输出的地方
		show_shell("当前目录无法创建更多的文件\n", lpBase);  //输出到show_shell
		UnmapViewOfFile(lpBase);  //关闭共享内存
		CloseHandle(hMapFile);
		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "无空闲i-结点\n";
		show_shell("无空闲i-结点\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < DIR_SIZE)
	{
		cout << "磁盘空间不足\n";
		show_shell("磁盘空间不足\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}
	for (i = 0; i < mdDir.iNode_number; i++)
	{
		if (dirName == iNode_table[mdDir.iNodes[i]].Name)
		{
			cout << "文件名重复，创建失败" << endl;
			show_shell("文件名重复\n", lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return;
		}
	}

	//分配空间
	newiNode = check_iNode();              //i-结点号
	newBlocks = check_blocks(DIR_SIZE);    //盘块开始号
	if (newBlocks == -1 || newiNode == -1) 
	{ 
		cout << "磁盘空间不足" << endl;
	}
	//cout << newiNode << endl;
	//cout << newBlocks << endl;
	//父目录
	mdDir.iNodes[mdDir.iNode_number] = newiNode;
	mdDir.iNode_number++;//父目录子i-结点增加

	//新目录
	Dir newDir;
	newDir.iNode = newiNode;
	newDir.iNode_number = 0;
	for (i = 0, j = newBlocks; i < DIR_SIZE; i++, j++) //盘块位图
	{
		blocks_bitmap[j] = USED;
	}
	//i-结点
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = DIR_SIZE;
	iNode_table[newiNode].fileType = DIRECTORY_TYPE;
	iNode_table[newiNode].Name = dirName;
	iNode_table[newiNode].size = sizeof(Dir);
	iNode_table[newiNode].father_iNode = mdDir.iNode;
	iNode_table[newiNode].uNumber = curUser;
	//超级块
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++) 
	{
		blockGroups[i].sBlock.all_freeBlocks -= DIR_SIZE; //空闲块总数减目录的块数
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //减目录的i结点
	}

	//写入磁盘
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	f_simdisk.seekp(iNode_table[mdDir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&mdDir), sizeof(Dir));
	f_simdisk.seekp(iNode_table[newDir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&newDir), sizeof(Dir));
	f_simdisk.close();
	cout << "创建成功" << endl;


	show_shell("创建成功\n", lpBase);

	UnmapViewOfFile(lpBase);
	CloseHandle(hMapFile);
}
void order_dir()
{
	cout << "文件或目录数：" << curDir.iNode_number << endl;
	int i;
	string filetype;
	cout << setw(15) << "文件类型" << setw(15) << "文件名" << setw(15) << "文件大小" << setw(15) << "文件所有者" << endl;
	for (i = 0; i < curDir.iNode_number; i++)
	{
		if (iNode_table[curDir.iNodes[i]].fileType == DIRECTORY_TYPE)
		{
			filetype = "目录";
		}
		else
		{
			filetype = "文件";
		}

		cout << setw(15) << filetype << setw(15) << iNode_table[curDir.iNodes[i]].Name << setw(10) << iNode_table[curDir.iNodes[i]].size << " B" << setw(15) << users[iNode_table[curDir.iNodes[i]].uNumber].uName << endl;
	}
}

void order_rd(string pathName)
{
	
	fstream f_simdisk;
	int i, iNode;
	Dir rdDir;
	i = find_by_pathName(pathName);
	if (users[curUser].uType <= users[iNode_table[i].uNumber].uType)
	{
		if (users[curUser].uName != users[iNode_table[i].uNumber].uName)
		{
			cout << "无操作权限" << endl;
			HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
			LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			show_shell("无操作权限\n", lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return;
		}
	}
	if (i > 0)
	{
		if (iNode_table[i].fileType == DIRECTORY_TYPE)
		{
			f_simdisk.open("simdisk", ios::in | ios::binary);
			f_simdisk.seekg(iNode_table[i].BlockAddr, ios::beg);
			f_simdisk.read(reinterpret_cast<char *>(&rdDir), sizeof(Dir));
			f_simdisk.close();
			if (rdDir.iNode_number > 0)
			{
				cout << "目录不为空是否确定删除（Y/N）" << endl;
				string a;
				cin >> a;
				if (a == "Y" || a == "y")
				{
					delete_file(rdDir.iNode);  //删除i-结点的目录
				}
				else
				{
					return;
				}
			}
			else
			{
				delete_file(rdDir.iNode);
			}
		}
		else
		{
			iNode = i;
			//盘块位图
			int bitmap = addr_to_bitmap(iNode_table[iNode].BlockAddr);
			for (i = 0; i < iNode_table[iNode].BlockNumber; i++, bitmap++)
			{
				blocks_bitmap[bitmap] = NOT_USED;
			}
			//超级块
			for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
			{
				blockGroups[i].sBlock.all_freeBlocks += iNode_table[iNode].BlockNumber; //空闲块总数
				blockGroups[i].sBlock.all_freeiNodes += 1;  //i结点总数
			}
			//i-结点位图
			iNode_bitmap[iNode] = NOT_USED;
			//父目录
			Dir f_Dir;
			f_simdisk.open("simdisk", ios::in | ios::binary);
			f_simdisk.seekg(iNode_table[iNode_table[iNode].father_iNode].BlockAddr, ios::beg);
			f_simdisk.read(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
			f_simdisk.close();
			for (i = 0; i < f_Dir.iNode_number; i++)
			{
				if (iNode_table[f_Dir.iNodes[i]].Name == iNode_table[iNode].Name)
				{
					f_Dir.iNodes[i] = f_Dir.iNodes[f_Dir.iNode_number - 1];
					break;
				}
			}
			f_Dir.iNode_number -= 1;
			f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
			f_simdisk.seekp(iNode_table[iNode_table[iNode].father_iNode].BlockAddr, ios::beg);
			f_simdisk.write(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
			f_simdisk.close();
		}
	}
	else
	{
		return;
	}
	//写入磁盘
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	f_simdisk.close();
	cout << "删除成功" << endl;
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	show_shell("删除成功\n", lpBase);
	UnmapViewOfFile(lpBase);
	CloseHandle(hMapFile);
}
void order_newfile(string filename, string content)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//文件内容为content
	int size = 0;
	string b;
	char c;
	if (content == "")
	{
		cout << "请输入文件内容：" << endl;
		while (cin >> b) //ctrl+z中止输入
		{
			c = cin.get();
			content += b;
			content += c;
		}
	}
	size = content.length();
	cin.clear();//避免ctrl+z后cin失效

	//文件大小
	int blocks;
	blocks = (size / 1024) + 1; //需要的盘块

	int i, newiNode, newBlocks, j;
	if (curDir.iNode_number == MAX_SUBFILE)
	{
		cout << "当前目录无法创建更多的文件\n";

		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "无空闲i-结点\n";
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < blocks)
	{
		cout << "磁盘空间不足\n";
		return;
	}
	for (i = 0; i < curDir.iNode_number; i++)
	{
		if (filename == iNode_table[curDir.iNodes[i]].Name)
		{
			cout << "文件名重复，创建失败" << endl;
			show_shell("文件名重复\n", lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return;
		}
	}

	//分配空间
	newiNode = check_iNode();              //i-结点号
	newBlocks = check_blocks(blocks);    //盘块开始号
	if (newBlocks == -1 || newiNode == -1)
	{
		cout << "磁盘空间不足" << endl;
	}
	for (i = 0, j = newBlocks; i < blocks; i++, j++) //盘块位图
	{
		blocks_bitmap[j] = USED;
	}
	//当前目录	
	curDir.iNodes[curDir.iNode_number] = newiNode;
	curDir.iNode_number += 1;



	//i-结点
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = blocks;
	iNode_table[newiNode].fileType = FILE_TYPE;
	iNode_table[newiNode].Name = filename;
	iNode_table[newiNode].size = size;
	iNode_table[newiNode].father_iNode = curDir.iNode;
	iNode_table[newiNode].uNumber = curUser;

	//超级块
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		blockGroups[i].sBlock.all_freeBlocks -= blocks; //空闲块总数减目录的块数
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //减目录的i结点
	}

	//写入磁盘
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	//父目录
	f_simdisk.seekp(iNode_table[curDir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&curDir), sizeof(Dir));
	//文件内容
	f_simdisk.seekp(iNode_table[newiNode].BlockAddr, ios::beg);

	
	char *str;
	str = new char[size];
	for (i = 0; i < size; i++)
	{
		str[i] = content[i];
	}
	
	f_simdisk.write(reinterpret_cast<char *>(str), size);
	f_simdisk.close();
	cout << "创建成功\n";
	show_shell("创建成功\n", lpBase);
	UnmapViewOfFile(lpBase);
	CloseHandle(hMapFile);
}
void order_cat(string pathName)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	int i, iNode, f_iNode;
	fstream f_simdisk;
	//文件i结点
	iNode = find_by_pathName(pathName);
	if (iNode < 0)
	{
		return;
	}
	if (users[curUser].uType < users[iNode_table[iNode].uNumber].uType)
	{
		cout << "无权限操作" << endl;
		show_shell("无权限操作\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}
	string filename = iNode_table[iNode].Name;
	//读取父目录
	f_iNode = iNode_table[iNode].father_iNode;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	Dir f_Dir;
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	//查找文件
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (iNode_table[f_Dir.iNodes[i]].Name == filename && iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE)//找到
		{
			//读取文件内容到str
			char *str;
			str = new char[iNode_table[f_Dir.iNodes[i]].size + 1];
			str[iNode_table[f_Dir.iNodes[i]].size] = '\0';
			f_simdisk.seekg(iNode_table[f_Dir.iNodes[i]].BlockAddr, ios::beg);
			f_simdisk.read(reinterpret_cast<char *>(str), iNode_table[f_Dir.iNodes[i]].size);
			f_simdisk.close();
			cout << str << endl;
			//在shell显示
			show_shell(str, lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return;
		}
	}
	f_simdisk.close();
}
void order_copy(string path1, string path2)
{
	//找要复制的文件
	int i, iNode, f_iNode, size;
	char *str1 = '\0';
	fstream f_simdisk;
	//文件i结点
	iNode = find_by_pathName(path1);
	if (iNode < 0)
	{
		return;
	}
	string filename = iNode_table[iNode].Name;
	//读取父目录
	f_iNode = iNode_table[iNode].father_iNode;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	Dir f_Dir;
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	//查找文件
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (iNode_table[f_Dir.iNodes[i]].Name == filename && iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE)//找到
		{
			//读取文件内容到str
			str1 = new char[iNode_table[f_Dir.iNodes[i]].size + 1];
			str1[iNode_table[f_Dir.iNodes[i]].size] = '\0';
			f_simdisk.seekg(iNode_table[f_Dir.iNodes[i]].BlockAddr, ios::beg);
			f_simdisk.read(reinterpret_cast<char *>(str1), iNode_table[f_Dir.iNodes[i]].size);
			size = iNode_table[f_Dir.iNodes[i]].size;
			break;
		}
	}
	if (i == f_Dir.iNode_number)
	{
		cout << "没有文件" << filename << endl;
		f_simdisk.close();
		return;
	}

	//复制到的目录
	f_iNode = find_by_pathName(path2);
	if (f_iNode < 0)
	{
		cout << "无目录" << path2 << endl;
		f_simdisk.close();
		return;
	}
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	
	//文件大小
	int blocks;
	blocks = (size / 1024) + 1; //需要的盘块

	int newiNode, newBlocks, j;
	if (f_Dir.iNode_number == MAX_SUBFILE)
	{
		cout << "目录无法创建更多的文件\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "无空闲i-结点\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < blocks)
	{
		cout << "磁盘空间不足\n";
		f_simdisk.close();
		return;
	}
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (filename == iNode_table[f_Dir.iNodes[i]].Name)
		{
			cout << "文件名重复，复制失败" << endl;
			f_simdisk.close();
			return;
		}
	}

	//分配空间
	newiNode = check_iNode();              //i-结点号
	newBlocks = check_blocks(blocks);    //盘块开始号
	if (newBlocks == -1 || newiNode == -1)
	{
		cout << "磁盘空间不足" << endl;
		f_simdisk.close();
		return;
	}
	for (i = 0, j = newBlocks; i < blocks; i++, j++) //盘块位图
	{
		blocks_bitmap[j] = USED;
	}
	//父目录	
	f_Dir.iNodes[f_Dir.iNode_number] = newiNode;
	f_Dir.iNode_number += 1;



	//i-结点
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = blocks;
	iNode_table[newiNode].fileType = FILE_TYPE;
	iNode_table[newiNode].Name = filename;
	iNode_table[newiNode].size = size;
	iNode_table[newiNode].father_iNode = f_Dir.iNode;

	//超级块
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		blockGroups[i].sBlock.all_freeBlocks -= blocks; //空闲块总数减目录的块数
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //减目录的i结点
	}
	f_simdisk.close();
	//写入磁盘
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	//父目录
	f_simdisk.seekp(iNode_table[f_Dir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
	//文件内容
	f_simdisk.seekp(iNode_table[newiNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(str1), size);
	f_simdisk.close();
	cout << "复制成功" << endl;
	//
}
void order_copy_host_to_sim(string path1, string path2)
{
	int i;
	string h;
	h = host_path(path1);
	//读取宿主机文件内容
	ifstream fin;
	fin.open(h, ios::in);
	if (!fin)
	{
		cout << "无法打开宿主机文件" << endl;
	}
	stringstream buf;
	buf << fin.rdbuf();
	//文件内容
	string content;
	content = buf.str();
	fin.close();
	string filename;//文件名
	filename = host_filename(h);

	//复制到simdisk
	fstream f_simdisk;
	f_simdisk.open("simdisk", ios::in, ios::binary);
	int f_iNode, size;
	Dir f_Dir;
	f_iNode = find_by_pathName(path2);
	if (f_iNode < 0)
	{
		cout << "无目录" << path2 << endl;
		f_simdisk.close();
		return;
	}
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);

	//文件大小
	size = content.length();
	int blocks;
	blocks = (size / 1024) + 1; //需要的盘块

	int newiNode, newBlocks, j;
	if (f_Dir.iNode_number == MAX_SUBFILE)
	{
		cout << "目录无法创建更多的文件\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "无空闲i-结点\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < blocks)
	{
		cout << "磁盘空间不足\n";
		f_simdisk.close();
		return;
	}
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (filename == iNode_table[f_Dir.iNodes[i]].Name)
		{
			cout << "文件名重复，复制失败" << endl;
			f_simdisk.close();
			return;
		}
	}

	//分配空间
	newiNode = check_iNode();              //i-结点号
	newBlocks = check_blocks(blocks);    //盘块开始号
	if (newBlocks == -1 || newiNode == -1)
	{
		cout << "磁盘空间不足" << endl;
		f_simdisk.close();
		return;
	}
	for (i = 0, j = newBlocks; i < blocks; i++, j++) //盘块位图
	{
		blocks_bitmap[j] = USED;
	}
	//父目录	
	f_Dir.iNodes[f_Dir.iNode_number] = newiNode;
	f_Dir.iNode_number += 1;



	//i-结点
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = blocks;
	iNode_table[newiNode].fileType = FILE_TYPE;
	iNode_table[newiNode].Name = filename;
	iNode_table[newiNode].size = size;
	iNode_table[newiNode].father_iNode = f_Dir.iNode;

	//超级块
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		blockGroups[i].sBlock.all_freeBlocks -= blocks; //空闲块总数减目录的块数
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //减目录的i结点
	}
	f_simdisk.close();
	//写入磁盘
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	//父目录
	f_simdisk.seekp(iNode_table[f_Dir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
	//文件内容
	f_simdisk.seekp(iNode_table[newiNode].BlockAddr, ios::beg);
	char *str;
	str = new char[size];
	for (i = 0; i < size; i++)
	{
		str[i] = content[i];
	}
	f_simdisk.write(reinterpret_cast<char *>(str), size);
	f_simdisk.close();
	cout << "复制成功" << endl;
}
void order_copy_sim_to_host(string path1, string path2)
{
	//文件内容
	int i, iNode, f_iNode, size;
	char *str1 = '\0';
	fstream f_simdisk;
	//文件i结点
	iNode = find_by_pathName(path1);
	if (iNode < 0)
	{
		return;
	}
	string filename = iNode_table[iNode].Name;
	//读取父目录
	f_iNode = iNode_table[iNode].father_iNode;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	Dir f_Dir;
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	//查找文件
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (iNode_table[f_Dir.iNodes[i]].Name == filename && iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE)//找到
		{
			//读取文件内容到str
			str1 = new char[iNode_table[f_Dir.iNodes[i]].size + 1];
			str1[iNode_table[f_Dir.iNodes[i]].size] = '\0';
			f_simdisk.seekg(iNode_table[f_Dir.iNodes[i]].BlockAddr, ios::beg);
			f_simdisk.read(reinterpret_cast<char *>(str1), iNode_table[f_Dir.iNodes[i]].size);
			size = iNode_table[f_Dir.iNodes[i]].size;
			break;
		}
	}
	if (i == f_Dir.iNode_number)
	{
		cout << "没有文件" << filename << endl;
		f_simdisk.close();
		return;
	}
	f_simdisk.close();

	//复制到宿主机
	string host_p;
	host_p = host_path(path2); //宿主机地址
	host_p += '\\';
	host_p += filename;
	f_simdisk.open(host_p, ios::out);
	f_simdisk << str1;  //写入内容
	f_simdisk.close();
	cout << "复制成功" << endl;
}
void order_check()
{
	int i;
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	f_simdisk.close();
	cout << "磁盘修复成功" << endl;
}

void process(string order)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //打开共享内存
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	share_read(sd, lpBase);
	if (order == "info")
	{
		order_info();
	}
	else if (order == "md")
	{
		if (sd.uNumber < 0 || sd.uNumber == curUser)
		{
			sd.uNumber = curUser;  //上锁
			share_write(sd, lpBase);  //上锁
			string dirName;  //执行命令
			cin >> dirName;//执行命令
			order_md(dirName, curDir);//执行命令
			sd.uNumber = -1;  //释放
			share_write(sd, lpBase); //释放
		}
		else
		{
			cout << "请等待其他进程写操作完成\n";
		}
	}
	else if (order == "cd")
	{
		string pathName;
		cin >> pathName;
		order_cd(pathName);
	}
	else if (order == "dir")
	{
		order_dir();
	}
	else if (order == "rd")
	{
		if (sd.uNumber < 0 || sd.uNumber == curUser)
		{
			sd.uNumber = curUser;
			share_write(sd, lpBase);
			string pathName;
			cin >> pathName;
			order_rd(pathName);
			read_info();
			sd.uNumber = -1;
			share_write(sd, lpBase);
		}
		else
		{
			cout << "请等待其他进程写操作完成\n";
		}


	}
	else if (order == "newfile")
	{
		if (sd.uNumber < 0 || sd.uNumber == curUser)  //判断是否有用户写
		{
			sd.uNumber = curUser;    //当前用户为写用户
			share_write(sd, lpBase);  //上锁
			string filename;    //执行命令
			cin >> filename;    //执行命令
			order_newfile(filename);//执行命令
			
			sd.uNumber = -1;    //写完后，修改uNumber
			share_write(sd, lpBase);//释放锁
		}
		else
		{
			cout << "请等待其他进程写操作完成\n";
		}
	}
	else if (order == "cat")
	{
		string pathName;
		cin >> pathName;
		order_cat(pathName);
	}
	else if (order == "del")
	{
		if (sd.uNumber < 0 || sd.uNumber == curUser)
		{
			sd.uNumber = curUser;
			share_write(sd, lpBase);
			string pathName;
			cin >> pathName;
			order_rd(pathName);
			read_info();
			sd.uNumber = -1;
			share_write(sd, lpBase);
		}
		else
		{
			cout << "请等待其他进程写操作完成\n";
		}
	
	}
	else if (order == "copy")
	{
		string path1, path2, str1, str2;
		cin >> path1 >> path2;
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
	else if (order == "adduser")
	{
		string uName, password, uType;
		cin >> uName >> password >> uType;
		if (users[curUser].uType == ROOT_USER)
		{
			
			add_user(uName, password, uType);
		}
		else
		{
			cout << "无权限操作" << endl;
		}
		
	}
	else if (order == "users")
	{
		order_users();
	}
	else if (order == "deluser")
	{
		string uName;
		cin >> uName;
		if (users[curUser].uType == ROOT_USER)
		{

			delete_user(uName);
		}
		else
		{
			cout << "无权限操作" << endl;
		}
	}
	else if (order == "check")
	{
		order_check();
	}
	else
	{
		cout << "无此命令" << endl;
	}
	// 解除文件映射
	UnmapViewOfFile(lpBase);
	// 关闭内存映射文件对象句柄
	CloseHandle(hMapFile);
}