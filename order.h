#pragma once
#include<string>
void process(string order);  //��������
//��������ĺ���
void order_info();
void order_cd(string pathName);
void order_md(string dirName, Dir &mdDir);
void order_dir();
void order_rd(string pathName);
void order_newfile(string filename, string content="");
void order_cat(string filename);
void order_copy(string, string);
void order_copy_host_to_sim(string, string);
void order_copy_sim_to_host(string, string);
void order_users();
void order_check();
