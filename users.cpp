#include"simdisk.h"
#include"function.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<sstream>
using namespace std;
int login()
{
	string u, p;
	cout << "�����û���������:\n";
	cin >> u >> p;
	int i;
	for (i = 0; i < MAX_USER; i++)//�����û��б�
	{
		if ((u == users[i].uName) && (p == users[i].password))
		{
			curUser = i;
			return 1;//��¼�ɹ�
		}
	}
	return -1; //�û��������ڻ��������

}
void add_user(string uName, string password, string uType)
{
	int i, u;
	for (i = 0; i < MAX_USER; i++)
	{
		if (users[i].uType == NOT_USED)
		{
			u = i;
			break;
		}
	}
	if (i == MAX_USER)
	{
		cout << "�û������Ѵ�����" << endl;
		return;
	}
	for (i = 0; i < MAX_USER; i++)
	{
		if (users[i].uName == uName)
		{
			cout << "�û����ظ�" << endl;
			return;
		}
	}

	if (uType == "user" || uType == "USER")
	{
		users[u].uType = USER;
	}
	else if (uType == "SYSTEM" || uType == "system")
	{
		users[u].uType = SYSTEM_USER;
	}
	else
	{
		cout << "�����ڵ��û�����" << endl;
	}	
	users[u].uName = uName;
	users[u].password = password;
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	cout << "����û��ɹ�" << endl;
	f_simdisk.close();
}
void delete_user(string uName)
{
	int i;
	for (i = 0; i < MAX_USER; i++)
	{
		if (users[i].uName == uName)
		{
			users[i].uType = NOT_USED;
			f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
			for (i = 0; i < MAX_USER; i++)
			{
				f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
			}
			f_simdisk.close();
			cout << "ɾ���ɹ�" << endl;
			return;
		}
	}
	cout << "�����ڵ��û�" << endl;
}
void order_users()
{
	string type;
	cout << "�û���Ϣ��" << endl;
	cout << setw(15) << "�û���" << setw(15) << "�û�����" << endl;
	int i;
	for (i = 0; i < MAX_USER; i++)
	{
		if (users[i].uType == 3)
		{
			type = "ROOT";
		}
		else if (users[i].uType == 2)
		{
			type = "SYSTEM";
		}
		else if (users[i].uType == 1)
		{
			type = "USER";
		}
		else
		{
			continue;
		}

		cout << setw(15) << users[i].uName << setw(15) << type << endl;
	}
}