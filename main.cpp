#include"simdisk.h"
#include"function.h"
#include"order.h"
#include<iostream>
#include<string>
#include<fstream>
#include <windows.h>
using namespace std;

int iNode_bitmap[INODE_NUMBER];  //i���λͼ
i_Node iNode_table[INODE_NUMBER];  //i����
int blocks_bitmap[BlOCKS];       //�̿�λͼ
BlockGroup blockGroups[BLOCK_GROUP_NUMBER];  //���ݿ��


Dir curDir;   //��ǰĿ¼
int curUser = -1;  //��ǰ�û�
fstream f_simdisk;    //��������ļ�
User users[MAX_USER];  //�û��б�


int BUF_SIZE = 4096;


shareData sd;
int main()
{	
	//cout << DATA_ADDR << endl;
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemorySZHC"); //�򿪹����ڴ�
	LPVOID lpBase;
	if (hMapFile == NULL)
	{// ���������ļ���� 
		HANDLE hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,   // �����ļ����
			NULL,   // Ĭ�ϰ�ȫ����
			PAGE_READWRITE,   // �ɶ���д
			0,   // ��λ�ļ���С
			BUF_SIZE,   // ��λ�ļ���С
			"ShareMemorySZHC"   // �����ڴ�����
		);

		// ӳ�仺������ͼ , �õ�ָ�����ڴ��ָ��
			lpBase = MapViewOfFile(
			hMapFile,            // �����ڴ�ľ��
			FILE_MAP_ALL_ACCESS, // �ɶ�д���
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
		cout << "��ģ���ļ�ϵͳ���ڣ��Ƿ��ʼ�� Y/N\n";
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
	read_info();  //��ȡ������Ϣ
	int m;  
	cout << "ѡ��ģʽ��0��̨ģʽ��1������ģʽ��" << endl;
	cin >> m;  //0��̨ģʽ��1������ģʽ
	sd.uNumber = -1;
	share_write(sd, lpBase);
	if (m == 0)
	{//��̨ģʽ

		sd.flag = 0;
		share_write(sd, lpBase);
		int i;
		while (1)
		{
			//��¼����
			Sleep(100);
			share_read(sd, lpBase);
			if (sd.flag == 1 && curUser == -1)
			{
				for (i = 0; i < MAX_USER; i++)
				{
					if ((sd.data1 == users[i].uName) && (sd.data2 == users[i].password))
					{
						curUser = i;
						break;//��¼�ɹ�
					}
				}
				if (i == MAX_USER)
				{
					cout << "�û������������" << endl;
					show_shell("�û������������", lpBase);
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
			while (1)  //�ȴ�shell������
			{
				share_read(sd, lpBase);
				if (sd.data3 == "")  //�ȴ�shell������
				{
					break;
				}
			}
			sd.data3 = get_path(curDir) + "/>"; //��ǰ·��
			sd.flag = 2;  //����flag��shell���
			share_write(sd, lpBase);  //��shell���·��
			while (1)//�ȴ�flag = 1
			{
				share_read(sd, lpBase);  //ѭ����ȡ�����ڴ�
				if (sd.flag == 1)
				{
					break;
				}
			}
			if (sd.flag == 1) //��������
			{
				process_shell(sd);
			}
			
		}
		// ����ļ�ӳ��
		UnmapViewOfFile(lpBase);
		// �ر��ڴ�ӳ���ļ�������
		CloseHandle(hMapFile);
	}
	else
	{//������ģʽ
		string order;
		while (curUser == -1)
		{
			int b = login();
			if (b == -1)
			{
				cout << "�û������ڻ��������" << endl;
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
		cout << "�޴�����" << endl;
	}
}