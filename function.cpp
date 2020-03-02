#include"simdisk.h"
#include"order.h"
#include"function.h"
#include<iostream>
#include<string>
#include<fstream>
#include<windows.h>
using namespace std;


void disk_init()
{
	int i;
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)  //��ʼ�����ݿ�
	{
		blockGroups[i].sBlock.all_freeBlocks = BlOCKS - DIR_SIZE; //���п���������Ŀ¼�Ŀ���
		blockGroups[i].sBlock.all_freeiNodes = INODE_NUMBER - 1;  //����Ŀ¼��i���
		blockGroups[i].BlockAddr = DATA_ADDR + i * BLOCK_SIZE * BLOCK_NUMBER;
		blockGroups[i].BlockMap = i * BLOCK_NUMBER;
		blockGroups[i].freeBlocks = BLOCK_NUMBER;
		blockGroups[i].freeiNode = BLOCK_NUMBER;
		blockGroups[i].iNodeMap = i * BLOCK_NUMBER;
		blockGroups[i].iNodeTable = i * BLOCK_NUMBER;
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		iNode_bitmap[i] = NOT_USED;  //��ʼ��i���λͼ
		blocks_bitmap[i] = NOT_USED;  //��ʼ���̿�λͼ
		//��ʼ��i����
		iNode_table[i].BlockAddr = 0;
		iNode_table[i].BlockNumber = 0;
		iNode_table[i].fileType = -1;
		iNode_table[i].Name = "new file";
		iNode_table[i].size = 0;
		iNode_table[i].father_iNode = -1;
	}
	//�û�
	for (i = 0; i < MAX_USER; i++)
	{
		users[i].uName = "";
		users[i].password = "";
		users[i].uType = NOT_USED;
	}
	//��ǰ�û�Ϊroot�û�
	users[0].uName = "root";
	users[0].password = "password";
	users[0].uType = ROOT_USER;
	

	//��ǰĿ¼Ϊ��Ŀ¼
	curDir.iNode = 0;
	curDir.iNode_number = 0;
	
	blockGroups[0].freeBlocks -= DIR_SIZE;
	blockGroups[0].freeiNode -= 1;
	for (i = 0; i < DIR_SIZE; i++)
	{
		blocks_bitmap[i] = USED;
	}
	iNode_bitmap[0] = USED;
	iNode_table[0].BlockAddr = DATA_ADDR;
	iNode_table[0].BlockNumber = DIR_SIZE;
	iNode_table[0].fileType = DIRECTORY_TYPE;
	iNode_table[0].Name = "root";
	iNode_table[0].size = sizeof(Dir);
	iNode_table[0].father_iNode = 0;
	iNode_table[0].uNumber = 0;


	//���浽�ļ�
	f_simdisk.open("simdisk", ios::out | ios::binary);
	if (!f_simdisk) {
		cout << "error_disk_init" << endl;
	}
	for (i = 0; i < MAX_USER; i++)  //�û��б�
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++) //���ݿ���
	{
		f_simdisk.write(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)  //i-���λͼ
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)  //i-����
	{
		f_simdisk.write(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)   //�̿�λͼ
	{
		f_simdisk.write(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	//��Ŀ¼
	f_simdisk.seekp(iNode_table[curDir.iNode].BlockAddr, ios::beg);
	f_simdisk.write(reinterpret_cast<char *>(&curDir), sizeof(Dir));
	//����100MB���ݿռ�
	int dataSize = BlOCKS * BLOCK_SIZE;
	char *buffer = new char[dataSize];
	f_simdisk.write(reinterpret_cast<char *>(buffer), sizeof(char) * dataSize);
	f_simdisk.close();
	delete(buffer);

}
void read_info()
{
	f_simdisk.open("simdisk", ios::in | ios::binary);
	if (!f_simdisk) {
		cout << "error_read_info" << endl;
	}
	int i;
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.read(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
	{
		f_simdisk.read(reinterpret_cast<char *>(&blockGroups[i]), sizeof(BlockGroup));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.read(reinterpret_cast<char *>(&iNode_bitmap[i]), sizeof(int));
	}
	for (i = 0; i < INODE_NUMBER; i++)
	{
		f_simdisk.read(reinterpret_cast<char *>(&iNode_table[i]), sizeof(i_Node));
	}
	for (i = 0; i < BlOCKS; i++)
	{
		f_simdisk.read(reinterpret_cast<char *>(&blocks_bitmap[i]), sizeof(int));
	}
	f_simdisk.seekg(iNode_table[curDir.iNode].BlockAddr, ios::beg);
	f_simdisk.read(reinterpret_cast<char *>(&curDir), sizeof(Dir));
	f_simdisk.close();

}

string get_path(Dir dir)  //��ȡĿ¼dir������·��
{
	int iNode;
	string path, curName, temp;
	iNode = dir.iNode;
	while (iNode)
	{
		curName = "/" + iNode_table[iNode].Name;
		temp = path;
		path = curName + temp;
		iNode = iNode_table[iNode].father_iNode;
	}//cout << 3 << endl;

	temp = path;
	path = "root" + temp;
	return path;
}

int check_iNode() //���ؿ��õ�i-����
{
	int i;
	for (i = 0; i < INODE_NUMBER; i++)
	{
		if (iNode_bitmap[i] == NOT_USED)  //��ʼ��i���λͼ
		{
			return i;
		}
	}
	return -1;
}
int check_blocks(int size) //���ؿ��õ��̿��
{
	int i, j = 0;
	for (i = 0; i < BlOCKS; i++)
	{
		if (blocks_bitmap[i] == NOT_USED)
		{
			j++;
			if (j == size)
			{
				return i + 1 - j;
			}
		}
		else
		{
			j = 0;
		}
	}
	return -1;
}
int find_by_pathName(string pathName)  
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
	LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pathName == "/") //��Ŀ¼
	{
		return 0;
	}
	fstream f_simdisk;
	f_simdisk.open("simdisk", ios::in | ios::binary);
	if (!f_simdisk) 
	{
		cout << "error_find_by_pathname" << endl;
	}

	int pos = 0, n = curDir.iNode_number, diriNode; //posΪĿ¼����ʼ��λ��
	Dir tempDir = curDir;
	string str;

	while (pathName.find('/', pos) != pathName.npos) //ѭ������/
	{
		str = pathName.substr(pos, pathName.find('/', pos) - pos);  //Ŀ¼����ʼλ��pos��/��֮�������ΪĿ¼��
		n = tempDir.iNode_number;   //��ǰĿ¼�����ļ�
		while (n)   //ѭ�����ҵ�ǰĿ¼���Ƿ�浽Ŀ¼��Ϊstr��Ŀ¼
		{
			if (iNode_table[tempDir.iNodes[n - 1]].Name == str)
			{
				diriNode = tempDir.iNodes[n - 1]; //���ҵ������¼i-����
				break;
			}
			n--;
		}
		if (n == 0) //Ŀ¼������
		{
			cout << "������Ŀ¼���ļ�" << str << endl;

			show_shell("�ļ�������\n", lpBase);
			UnmapViewOfFile(lpBase);
			CloseHandle(hMapFile);
			return -1;
		}
		//�����������i-���ŴӴ��̶�ȡĿ¼Ϊ��ǰĿ¼
		f_simdisk.seekg(iNode_table[diriNode].BlockAddr, ios::beg);
		f_simdisk.read(reinterpret_cast<char *>(&tempDir), sizeof(Dir));
		//��һ��Ŀ¼����ʼ��λ��Ϊ/����һ���ַ�
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

	if (n == 0)
	{
		cout << "������Ŀ¼���ļ�" << str << endl;
		show_shell("�ļ�������\n", lpBase);
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapFile);
		return -1;
	}

	
	return diriNode;

	f_simdisk.close();
}
int addr_to_bitmap(int addr)
{
	return (addr - DATA_ADDR) / BLOCK_SIZE;
}
void delete_file(int iNode)
{
	int i;
	if (iNode_table[iNode].fileType == FILE_TYPE)
	{
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
	else
	{
		Dir rdDir;
		f_simdisk.open("simdisk", ios::in | ios::binary);
		f_simdisk.seekg(iNode_table[iNode].BlockAddr, ios::beg);
		f_simdisk.read(reinterpret_cast<char *>(&rdDir), sizeof(Dir));
		f_simdisk.close();

		if (rdDir.iNode_number == 0)
		{
			//�̿�λͼ
			int bitmap = addr_to_bitmap(iNode_table[rdDir.iNode].BlockAddr);
			for (i = 0; i < iNode_table[rdDir.iNode].BlockNumber; i++, bitmap++)
			{
				blocks_bitmap[bitmap] = NOT_USED;
			}
			//������
			for (i = 0; i < BLOCK_GROUP_NUMBER; i++)
			{
				blockGroups[i].sBlock.all_freeBlocks += iNode_table[rdDir.iNode].BlockNumber; //���п�����
				blockGroups[i].sBlock.all_freeiNodes += 1;  //i�������
			}
			//i-���λͼ
			iNode_bitmap[rdDir.iNode] = NOT_USED;
			//��Ŀ¼
			Dir f_Dir;
			f_simdisk.open("simdisk", ios::in | ios::binary);
			f_simdisk.seekg(iNode_table[iNode_table[rdDir.iNode].father_iNode].BlockAddr, ios::beg);
			f_simdisk.read(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
			f_simdisk.close();
			for (i = 0; i < f_Dir.iNode_number; i++)
			{
				if (iNode_table[f_Dir.iNodes[i]].Name == iNode_table[rdDir.iNode].Name)
				{
					f_Dir.iNodes[i] = f_Dir.iNodes[f_Dir.iNode_number - 1];
					break;
				}
			}
			f_Dir.iNode_number -= 1;
			f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
			f_simdisk.seekp(iNode_table[iNode_table[rdDir.iNode].father_iNode].BlockAddr, ios::beg);
			f_simdisk.write(reinterpret_cast<char *>(&f_Dir), sizeof(Dir));
			f_simdisk.close();
		}
		else
		{
			//�ݹ�ɾ��
			int n;
			n = rdDir.iNode_number; //��Ŀ¼��
			for (i = 0; i < n; i++)
			{
				delete_file(rdDir.iNodes[i]);
			}
			delete_file(rdDir.iNode);//��Ŀ¼ɾ����ɺ�ɾ���Լ�
		}
	}
}
string host_path(string pathName)
{
	string str, last, first;
	char a = '\\';
	str = pathName.substr(6);

	if (str[2] != '\\')
	{
		first = str.substr(0, 2);
		last = str.substr(2);
		first += a;
		first += last;
		return first;
	}
	else
	{
		return str;
	}

}
string host_filename(string pathName)
{
	int i = 1, pos = 2;
	while (1)
	{
		i = pathName.find('\\', pos);
		if (i == pathName.npos)
		{
			break;
		}
		i++;
		pos = i;
	}

	string str;
	str = pathName.substr(pos);
	return str;
}

void share_read(shareData &data, LPVOID lpBase)
{
	memcpy(&data, lpBase, sizeof(shareData));
}
void share_write(shareData& data, LPVOID lpBase)
{
	memcpy(lpBase, &data, sizeof(shareData));
}


