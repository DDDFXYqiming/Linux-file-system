#pragma once
#include<windows.h>
#include<string>
using namespace std;

#define BLOCK_SIZE          1024  //�̿��СΪ1KB
#define BLOCK_NUMBER        1024  //1024���̿�������ݿ�
#define BLOCK_GROUP_NUMBER  100   //100�����ݿ���ɴ���
#define BlOCKS              (BLOCK_NUMBER * BLOCK_GROUP_NUMBER)  //�̿�����

#define INODE_NUMBER        (BLOCK_NUMBER * BLOCK_GROUP_NUMBER)  //ÿ���̿鶼��i���

#define MAX_SUBFILE         300   //Ŀ¼��������ļ���Ŀ

#define DIR_SIZE            (sizeof(Dir)/BLOCK_SIZE + 1)  //һ��Ŀ¼��Ҫ���̿���

//λͼ
#define USED                1   
#define NOT_USED            0


//i�������
#define DIRECTORY_TYPE      0            //Ŀ¼
#define FILE_TYPE           1            //�ļ�

//�û�����
#define ROOT_USER           3         
#define SYSTEM_USER         2
#define USER                1


//�ļ�����
#define SHARED              0
#define READ_ONLY           1
#define READ_WRITE          2
#define SYSTEM              3

//����û�����
#define MAX_USER            10

//��ʼ��ַ
#define DATA_ADDR           (sizeof(BlockGroup) * BLOCK_GROUP_NUMBER + sizeof(int) * (INODE_NUMBER + BlOCKS) + sizeof(i_Node) * INODE_NUMBER + sizeof(User) * MAX_USER)

//�û���
class User      
{
public:
	string uName;  //�û���
	string password; //����
	int uType;   //�û�����
};

//Ŀ¼��
class Dir        
{
public:
	int iNode;      //i��������
	int iNode_number; //���ļ�i�������
	int iNodes[MAX_SUBFILE];      //���ļ�i������� 

};

//������
class SuperBlock
{
public:
	int all_freeBlocks;   //�����̿�����
	int all_freeiNodes;   //����i�������

};

//���ݿ�
class BlockGroup
{
public:
	SuperBlock sBlock;  //������
	//���ݿ���Ϣ
	int BlockMap;    //���ݿ鿪ʼλ�õ�λͼ����ֵ
	int BlockAddr;   //�׵�ַ	
	int iNodeMap;    //i���λͼ����
	int iNodeTable;  //i��������
	int freeiNode;   //����i���
	int freeBlocks;  //�����̿�����

};

//i-���
class i_Node
{
public:
	string Name;   //�ļ���
	int uNumber;    //�ļ�����������
	int fileType;    //�ļ�����
	int size;        //�ļ���С
	int BlockNumber; //ռ���̿���
	int BlockAddr;   //��ʼ��ַ
	int father_iNode;//��һ��Ŀ¼i�������
};

struct shareData
{
public:
	int flag;
	int uNumber;
	string order;
	string data1; //����1
	string data2; //����2
	string data3; //���
};


extern int iNode_bitmap[INODE_NUMBER];              //i���λͼ
extern i_Node iNode_table[INODE_NUMBER];            //i����
extern int blocks_bitmap[BlOCKS];                   //�̿�λͼ
extern BlockGroup blockGroups[BLOCK_GROUP_NUMBER];  //���ݿ��


extern Dir curDir;   //��ǰĿ¼
extern int curUser;  //��ǰ�û�����
extern fstream f_simdisk;    //��������ļ�

extern User users[MAX_USER];



extern shareData sd;
