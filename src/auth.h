#pragma once

#include "simdisk.h"
#include <iostream>
#include <string>

// 控制台登录，成功返回 1，失败返回 -1
int login(std::istream& in, std::ostream& out);
// root 添加用户
void add_user(const std::string& uName, const std::string& password, const std::string& uType, std::ostream& out);
// root 删除用户
void delete_user(const std::string& uName, std::ostream& out);
// 用户列表
void order_users(std::ostream& out);
