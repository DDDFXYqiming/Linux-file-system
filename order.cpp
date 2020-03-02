#include"simdisk.h"
#include"order.h"
#include"function.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<sstream>
using namespace std;

void order_info()      //info����
{
	cout << " ϵͳ��Ϣ\n";
	cout << setw(15) << "  �����ܿռ䣺 " << 100 * 1024 * 1024 << "B" << endl;
	cout << setw(15) << "  ���ÿռ䣺 " << 100 * 1024 * 1024 - blockGroups[0].sBlock.all_freeBlocks * 1024 << " B" << endl;
	cout << setw(15) << "  ʣ��ռ䣺 " << blockGroups[0].sBlock.all_freeBlocks * 1024 << " B" << endl;

	cout << "\n ���ݿ���Ϣ\n";
	cout << setw(15) << "  ���ݿ������� " << "100" << endl;
	cout << setw(15) << "  ÿ���̿����� " << "1024" << endl;

	cout << "\n �̿���Ϣ\n";
	cout << setw(15) << "  �̿��С�� " << "1024 B" << endl;
	cout << setw(15) << "  �̿������� " << "102400" << endl;
	cout << setw(15) << "  �����̿飺 " << 102400 - blockGroups[0].sBlock.all_freeBlocks << endl;
	cout << setw(15) << "  ʣ���̿����� " << blockGroups[0].sBlock.all_freeBlocks << endl;
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


	if (pathName == "..") //��һ��Ŀ¼
	{
		diriNode = iNode_table[curDir.iNode].father_iNode; //��ȡ��һ��Ŀ¼i-���
		f_simdisk.seekg(iNode_table[diriNode].BlockAddr, ios::beg);
		f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));
		f_simdisk.close();
		return;
	}
	if (pathName == "/")  //��Ŀ¼
	{
		f_simdisk.seekg(iNode_table[0].BlockAddr, ios::beg);
		f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));
		f_simdisk.close();
		return;
	}
	//ѭ������·��
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
			HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
			LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			cout << "������Ŀ¼���ļ�" << str << endl;
			show_shell("Ŀ¼������\n", lpBase);
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
		cout << "������Ŀ¼���ļ�" << str << endl;
		HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
		LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		show_shell("Ŀ¼������\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}

	//�ҵ�Ҫ����Ŀ¼��i-���󣬶�ȡ���̣���ֵ��curDir
	f_simdisk.seekg(iNode_table[diriNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));

	f_simdisk.close();
}
void order_md(string dirName, Dir &mdDir)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	string shell;
	int i, newiNode, newBlocks, j;
	//�жϿռ��Ƿ��㹻
	if (mdDir.iNode_number == MAX_SUBFILE)
	{
		cout << "��ǰĿ¼�޷�����������ļ�\n";  //ԭ������ĵط�
		show_shell("��ǰĿ¼�޷�����������ļ�\n", lpBase);  //�����show_shell
		UnmapViewOfFile(lpBase);  //�رչ����ڴ�
		CloseHandle(hMapFile);
		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "�޿���i-���\n";
		show_shell("�޿���i-���\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < DIR_SIZE)
	{
		cout << "���̿ռ䲻��\n";
		show_shell("���̿ռ䲻��\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}
	for (i = 0; i < mdDir.iNode_number; i++)
	{
		if (dirName == iNode_table[mdDir.iNodes[i]].Name)
		{
			cout << "�ļ����ظ�������ʧ��" << endl;
			show_shell("�ļ����ظ�\n", lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return;
		}
	}

	//����ռ�
	newiNode = check_iNode();              //i-����
	newBlocks = check_blocks(DIR_SIZE);    //�̿鿪ʼ��
	if (newBlocks == -1 || newiNode == -1) 
	{ 
		cout << "���̿ռ䲻��" << endl;
	}
	//cout << newiNode << endl;
	//cout << newBlocks << endl;
	//��Ŀ¼
	mdDir.iNodes[mdDir.iNode_number] = newiNode;
	mdDir.iNode_number++;//��Ŀ¼��i-�������

	//��Ŀ¼
	Dir newDir;
	newDir.iNode = newiNode;
	newDir.iNode_number = 0;
	for (i = 0, j = newBlocks; i < DIR_SIZE; i++, j++) //�̿�λͼ
	{
		blocks_bitmap[j] = USED;
	}
	//i-���
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = DIR_SIZE;
	iNode_table[newiNode].fileType = DIRECTORY_TYPE;
	iNode_table[newiNode].Name = dirName;
	iNode_table[newiNode].size = sizeof(Dir);
	iNode_table[newiNode].father_iNode = mdDir.iNode;
	iNode_table[newiNode].uNumber = curUser;
	//������
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++) 
	{
		blockGroups[i].sBlock.all_freeBlocks -= DIR_SIZE; //���п�������Ŀ¼�Ŀ���
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //��Ŀ¼��i���
	}

	//д�����
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
	cout << "�����ɹ�" << endl;


	show_shell("�����ɹ�\n", lpBase);

	UnmapViewOfFile(lpBase);
	CloseHandle(hMapFile);
}
void order_dir()
{
	cout << "�ļ���Ŀ¼����" << curDir.iNode_number << endl;
	int i;
	string filetype;
	cout << setw(15) << "�ļ�����" << setw(15) << "�ļ���" << setw(15) << "�ļ���С" << setw(15) << "�ļ�������" << endl;
	for (i = 0; i < curDir.iNode_number; i++)
	{
		if (iNode_table[curDir.iNodes[i]].fileType == DIRECTORY_TYPE)
		{
			filetype = "Ŀ¼";
		}
		else
		{
			filetype = "�ļ�";
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
			cout << "�޲���Ȩ��" << endl;
			HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
			LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			show_shell("�޲���Ȩ��\n", lpBase);
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
				cout << "Ŀ¼��Ϊ���Ƿ�ȷ��ɾ����Y/N��" << endl;
				string a;
				cin >> a;
				if (a == "Y" || a == "y")
				{
					delete_file(rdDir.iNode);  //ɾ��i-����Ŀ¼
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
			//�̿�λͼ
			int bitmap = addr_to_bitmap(iNode_table[iNode].BlockAddr);
			for (i = 0; i < iNode_table[iNode].BlockNumber; i++, bitmap++)
			{
				blocks_bitmap[bitmap] = NOT_USED;
			}
			//������
			for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
			{
				blockGroups[i].sBlock.all_freeBlocks += iNode_table[iNode].BlockNumber; //���п�����
				blockGroups[i].sBlock.all_freeiNodes += 1;  //i�������
			}
			//i-���λͼ
			iNode_bitmap[iNode] = NOT_USED;
			//��Ŀ¼
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
	//д�����
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
	cout << "ɾ���ɹ�" << endl;
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	show_shell("ɾ���ɹ�\n", lpBase);
	UnmapViewOfFile(lpBase);
	CloseHandle(hMapFile);
}
void order_newfile(string filename, string content)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//�ļ�����Ϊcontent
	int size = 0;
	string b;
	char c;
	if (content == "")
	{
		cout << "�������ļ����ݣ�" << endl;
		while (cin >> b) //ctrl+z��ֹ����
		{
			c = cin.get();
			content += b;
			content += c;
		}
	}
	size = content.length();
	cin.clear();//����ctrl+z��cinʧЧ

	//�ļ���С
	int blocks;
	blocks = (size / 1024) + 1; //��Ҫ���̿�

	int i, newiNode, newBlocks, j;
	if (curDir.iNode_number == MAX_SUBFILE)
	{
		cout << "��ǰĿ¼�޷�����������ļ�\n";

		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "�޿���i-���\n";
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < blocks)
	{
		cout << "���̿ռ䲻��\n";
		return;
	}
	for (i = 0; i < curDir.iNode_number; i++)
	{
		if (filename == iNode_table[curDir.iNodes[i]].Name)
		{
			cout << "�ļ����ظ�������ʧ��" << endl;
			show_shell("�ļ����ظ�\n", lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return;
		}
	}

	//����ռ�
	newiNode = check_iNode();              //i-����
	newBlocks = check_blocks(blocks);    //�̿鿪ʼ��
	if (newBlocks == -1 || newiNode == -1)
	{
		cout << "���̿ռ䲻��" << endl;
	}
	for (i = 0, j = newBlocks; i < blocks; i++, j++) //�̿�λͼ
	{
		blocks_bitmap[j] = USED;
	}
	//��ǰĿ¼	
	curDir.iNodes[curDir.iNode_number] = newiNode;
	curDir.iNode_number += 1;



	//i-���
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = blocks;
	iNode_table[newiNode].fileType = FILE_TYPE;
	iNode_table[newiNode].Name = filename;
	iNode_table[newiNode].size = size;
	iNode_table[newiNode].father_iNode = curDir.iNode;
	iNode_table[newiNode].uNumber = curUser;

	//������
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		blockGroups[i].sBlock.all_freeBlocks -= blocks; //���п�������Ŀ¼�Ŀ���
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //��Ŀ¼��i���
	}

	//д�����
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
	//��Ŀ¼
	f_simdisk.seekp(iNode_table[curDir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&curDir), sizeof(Dir));
	//�ļ�����
	f_simdisk.seekp(iNode_table[newiNode].BlockAddr, ios::beg);

	
	char *str;
	str = new char[size];
	for (i = 0; i < size; i++)
	{
		str[i] = content[i];
	}
	
	f_simdisk.write(reinterpret_cast<char *>(str), size);
	f_simdisk.close();
	cout << "�����ɹ�\n";
	show_shell("�����ɹ�\n", lpBase);
	UnmapViewOfFile(lpBase);
	CloseHandle(hMapFile);
}
void order_cat(string pathName)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	int i, iNode, f_iNode;
	fstream f_simdisk;
	//�ļ�i���
	iNode = find_by_pathName(pathName);
	if (iNode < 0)
	{
		return;
	}
	if (users[curUser].uType < users[iNode_table[iNode].uNumber].uType)
	{
		cout << "��Ȩ�޲���" << endl;
		show_shell("��Ȩ�޲���\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return;
	}
	string filename = iNode_table[iNode].Name;
	//��ȡ��Ŀ¼
	f_iNode = iNode_table[iNode].father_iNode;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	Dir f_Dir;
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	//�����ļ�
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (iNode_table[f_Dir.iNodes[i]].Name == filename && iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE)//�ҵ�
		{
			//��ȡ�ļ����ݵ�str
			char *str;
			str = new char[iNode_table[f_Dir.iNodes[i]].size + 1];
			str[iNode_table[f_Dir.iNodes[i]].size] = '\0';
			f_simdisk.seekg(iNode_table[f_Dir.iNodes[i]].BlockAddr, ios::beg);
			f_simdisk.read(reinterpret_cast<char *>(str), iNode_table[f_Dir.iNodes[i]].size);
			f_simdisk.close();
			cout << str << endl;
			//��shell��ʾ
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
	//��Ҫ���Ƶ��ļ�
	int i, iNode, f_iNode, size;
	char *str1 = '\0';
	fstream f_simdisk;
	//�ļ�i���
	iNode = find_by_pathName(path1);
	if (iNode < 0)
	{
		return;
	}
	string filename = iNode_table[iNode].Name;
	//��ȡ��Ŀ¼
	f_iNode = iNode_table[iNode].father_iNode;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	Dir f_Dir;
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	//�����ļ�
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (iNode_table[f_Dir.iNodes[i]].Name == filename && iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE)//�ҵ�
		{
			//��ȡ�ļ����ݵ�str
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
		cout << "û���ļ�" << filename << endl;
		f_simdisk.close();
		return;
	}

	//���Ƶ���Ŀ¼
	f_iNode = find_by_pathName(path2);
	if (f_iNode < 0)
	{
		cout << "��Ŀ¼" << path2 << endl;
		f_simdisk.close();
		return;
	}
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	
	//�ļ���С
	int blocks;
	blocks = (size / 1024) + 1; //��Ҫ���̿�

	int newiNode, newBlocks, j;
	if (f_Dir.iNode_number == MAX_SUBFILE)
	{
		cout << "Ŀ¼�޷�����������ļ�\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "�޿���i-���\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < blocks)
	{
		cout << "���̿ռ䲻��\n";
		f_simdisk.close();
		return;
	}
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (filename == iNode_table[f_Dir.iNodes[i]].Name)
		{
			cout << "�ļ����ظ�������ʧ��" << endl;
			f_simdisk.close();
			return;
		}
	}

	//����ռ�
	newiNode = check_iNode();              //i-����
	newBlocks = check_blocks(blocks);    //�̿鿪ʼ��
	if (newBlocks == -1 || newiNode == -1)
	{
		cout << "���̿ռ䲻��" << endl;
		f_simdisk.close();
		return;
	}
	for (i = 0, j = newBlocks; i < blocks; i++, j++) //�̿�λͼ
	{
		blocks_bitmap[j] = USED;
	}
	//��Ŀ¼	
	f_Dir.iNodes[f_Dir.iNode_number] = newiNode;
	f_Dir.iNode_number += 1;



	//i-���
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = blocks;
	iNode_table[newiNode].fileType = FILE_TYPE;
	iNode_table[newiNode].Name = filename;
	iNode_table[newiNode].size = size;
	iNode_table[newiNode].father_iNode = f_Dir.iNode;

	//������
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		blockGroups[i].sBlock.all_freeBlocks -= blocks; //���п�������Ŀ¼�Ŀ���
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //��Ŀ¼��i���
	}
	f_simdisk.close();
	//д�����
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
	//��Ŀ¼
	f_simdisk.seekp(iNode_table[f_Dir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
	//�ļ�����
	f_simdisk.seekp(iNode_table[newiNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(str1), size);
	f_simdisk.close();
	cout << "���Ƴɹ�" << endl;
	//
}
void order_copy_host_to_sim(string path1, string path2)
{
	int i;
	string h;
	h = host_path(path1);
	//��ȡ�������ļ�����
	ifstream fin;
	fin.open(h, ios::in);
	if (!fin)
	{
		cout << "�޷����������ļ�" << endl;
	}
	stringstream buf;
	buf << fin.rdbuf();
	//�ļ�����
	string content;
	content = buf.str();
	fin.close();
	string filename;//�ļ���
	filename = host_filename(h);

	//���Ƶ�simdisk
	fstream f_simdisk;
	f_simdisk.open("simdisk", ios::in, ios::binary);
	int f_iNode, size;
	Dir f_Dir;
	f_iNode = find_by_pathName(path2);
	if (f_iNode < 0)
	{
		cout << "��Ŀ¼" << path2 << endl;
		f_simdisk.close();
		return;
	}
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);

	//�ļ���С
	size = content.length();
	int blocks;
	blocks = (size / 1024) + 1; //��Ҫ���̿�

	int newiNode, newBlocks, j;
	if (f_Dir.iNode_number == MAX_SUBFILE)
	{
		cout << "Ŀ¼�޷�����������ļ�\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeiNodes == 0)
	{
		cout << "�޿���i-���\n";
		f_simdisk.close();
		return;
	}
	if (blockGroups[0].sBlock.all_freeBlocks < blocks)
	{
		cout << "���̿ռ䲻��\n";
		f_simdisk.close();
		return;
	}
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (filename == iNode_table[f_Dir.iNodes[i]].Name)
		{
			cout << "�ļ����ظ�������ʧ��" << endl;
			f_simdisk.close();
			return;
		}
	}

	//����ռ�
	newiNode = check_iNode();              //i-����
	newBlocks = check_blocks(blocks);    //�̿鿪ʼ��
	if (newBlocks == -1 || newiNode == -1)
	{
		cout << "���̿ռ䲻��" << endl;
		f_simdisk.close();
		return;
	}
	for (i = 0, j = newBlocks; i < blocks; i++, j++) //�̿�λͼ
	{
		blocks_bitmap[j] = USED;
	}
	//��Ŀ¼	
	f_Dir.iNodes[f_Dir.iNode_number] = newiNode;
	f_Dir.iNode_number += 1;



	//i-���
	iNode_bitmap[newiNode] = USED;
	iNode_table[newiNode].BlockAddr = DATA_ADDR + (newBlocks * BLOCK_SIZE);
	iNode_table[newiNode].BlockNumber = blocks;
	iNode_table[newiNode].fileType = FILE_TYPE;
	iNode_table[newiNode].Name = filename;
	iNode_table[newiNode].size = size;
	iNode_table[newiNode].father_iNode = f_Dir.iNode;

	//������
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		blockGroups[i].sBlock.all_freeBlocks -= blocks; //���п�������Ŀ¼�Ŀ���
		blockGroups[i].sBlock.all_freeiNodes -= 1;  //��Ŀ¼��i���
	}
	f_simdisk.close();
	//д�����
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
	//��Ŀ¼
	f_simdisk.seekp(iNode_table[f_Dir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
	//�ļ�����
	f_simdisk.seekp(iNode_table[newiNode].BlockAddr, ios::beg);
	char *str;
	str = new char[size];
	for (i = 0; i < size; i++)
	{
		str[i] = content[i];
	}
	f_simdisk.write(reinterpret_cast<char *>(str), size);
	f_simdisk.close();
	cout << "���Ƴɹ�" << endl;
}
void order_copy_sim_to_host(string path1, string path2)
{
	//�ļ�����
	int i, iNode, f_iNode, size;
	char *str1 = '\0';
	fstream f_simdisk;
	//�ļ�i���
	iNode = find_by_pathName(path1);
	if (iNode < 0)
	{
		return;
	}
	string filename = iNode_table[iNode].Name;
	//��ȡ��Ŀ¼
	f_iNode = iNode_table[iNode].father_iNode;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, ios::beg);
	Dir f_Dir;
	f_simdisk.read(reinterpret_cast<char *>(&f_Dir), iNode_table[f_iNode].size);
	//�����ļ�
	for (i = 0; i < f_Dir.iNode_number; i++)
	{
		if (iNode_table[f_Dir.iNodes[i]].Name == filename && iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE)//�ҵ�
		{
			//��ȡ�ļ����ݵ�str
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
		cout << "û���ļ�" << filename << endl;
		f_simdisk.close();
		return;
	}
	f_simdisk.close();

	//���Ƶ�������
	string host_p;
	host_p = host_path(path2); //��������ַ
	host_p += '\\';
	host_p += filename;
	f_simdisk.open(host_p, ios::out);
	f_simdisk << str1;  //д������
	f_simdisk.close();
	cout << "���Ƴɹ�" << endl;
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
	cout << "�����޸��ɹ�" << endl;
}

void process(string order)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
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
			sd.uNumber = curUser;  //����
			share_write(sd, lpBase);  //����
			string dirName;  //ִ������
			cin >> dirName;//ִ������
			order_md(dirName, curDir);//ִ������
			sd.uNumber = -1;  //�ͷ�
			share_write(sd, lpBase); //�ͷ�
		}
		else
		{
			cout << "��ȴ���������д�������\n";
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
			cout << "��ȴ���������д�������\n";
		}


	}
	else if (order == "newfile")
	{
		if (sd.uNumber < 0 || sd.uNumber == curUser)  //�ж��Ƿ����û�д
		{
			sd.uNumber = curUser;    //��ǰ�û�Ϊд�û�
			share_write(sd, lpBase);  //����
			string filename;    //ִ������
			cin >> filename;    //ִ������
			order_newfile(filename);//ִ������
			
			sd.uNumber = -1;    //д����޸�uNumber
			share_write(sd, lpBase);//�ͷ���
		}
		else
		{
			cout << "��ȴ���������д�������\n";
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
			cout << "��ȴ���������д�������\n";
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
			cout << "��Ȩ�޲���" << endl;
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
			cout << "��Ȩ�޲���" << endl;
		}
	}
	else if (order == "check")
	{
		order_check();
	}
	else
	{
		cout << "�޴�����" << endl;
	}
	// ����ļ�ӳ��
	UnmapViewOfFile(lpBase);
	// �ر��ڴ�ӳ���ļ�������
	CloseHandle(hMapFile);
}