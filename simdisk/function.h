#pragma once
#include<windows.h>
void disk_init();       //��ʼ��
void read_info();       //��ȡ������Ϣ
string get_path(Dir);   //����Ŀ¼������·��
int check_iNode();      //���ؿ��õ�i-����
int check_blocks(int size);   //���ؿ��õ��̿��
int find_by_pathName(string pathName);  //����·�������������ļ���Ŀ¼��i����
int addr_to_bitmap(int);   //��ַ���̿�ŵ�ת��
void delete_file(int);     //ɾ���ļ�
string host_path(string pathName);    //������·��ת��
string host_filename(string pathName);  //����·����ȡ�������ļ���
int login();   //��¼
void add_user(string uName, string password, string uType);   //����û�
void delete_user(string uName);   //ɾ���û�

void share_read(shareData&, LPVOID);
void share_write(shareData&, LPVOID);
void process_shell(shareData sd);
void show_shell(string, LPVOID);