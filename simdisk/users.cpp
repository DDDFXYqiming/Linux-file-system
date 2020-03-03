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
	cout << "输入用户名和密码:\n";
	cin >> u >> p;
	int i;
	for (i = 0; i < MAX_USER; i++)//遍历用户列表
	{
		if ((u == users[i].uName) && (p == users[i].password))
		{
			curUser = i;
			return 1;//登录成功
		}
	}
	return -1; //用户名不存在或密码错误

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
		cout << "用户数量已达上限" << endl;
		return;
	}
	for (i = 0; i < MAX_USER; i++)
	{
		if (users[i].uName == uName)
		{
			cout << "用户名重复" << endl;
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
		cout << "不存在的用户类型" << endl;
	}	
	users[u].uName = uName;
	users[u].password = password;
	f_simdisk.open("simdisk", ios::out | ios::binary | ios::_Nocreate);
	for (i = 0; i < MAX_USER; i++)
	{
		f_simdisk.write(reinterpret_cast<char *>(&users[i]), sizeof(User));
	}
	cout << "添加用户成功" << endl;
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
			cout << "删除成功" << endl;
			return;
		}
	}
	cout << "不存在的用户" << endl;
}
void order_users()
{
	string type;
	cout << "用户信息：" << endl;
	cout << setw(15) << "用户名" << setw(15) << "用户类型" << endl;
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