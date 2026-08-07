#pragma once

#include "simdisk.h"
#include <iostream>
#include <string>

// 命令集：输出统一走 ostream，交互输入走 istream（CLI 与 server 模式复用）
void order_info(std::ostream& out);
void order_cd(const std::string& pathName);
void order_md(const std::string& dirName, Dir& mdDir, std::ostream& out);
void order_dir(std::ostream& out);
void order_rd(const std::string& pathName, std::ostream& out, std::istream& in);
void order_newfile(const std::string& filename, std::ostream& out, std::istream& in, const std::string& content = "");
void order_cat(const std::string& pathName, std::ostream& out);
void order_copy(const std::string& path1, const std::string& path2, std::ostream& out);
void order_copy_host_to_sim(const std::string& path1, const std::string& path2, std::ostream& out);
void order_copy_sim_to_host(const std::string& path1, const std::string& path2, std::ostream& out);
void order_check();

// CLI 命令分发：读入并执行一条命令
void process(std::ostream& out, std::istream& in, const std::string& order);
