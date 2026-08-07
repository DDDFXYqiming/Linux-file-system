#include "auth.h"
#include "commands.h"
#include "filesystem.h"

#include <fstream>
#include <iostream>

// 控制台模式：单进程交互式命令解释器
int main() {
    std::fstream probe("simdisk", std::ios::in | std::ios::binary);
    if (!probe.is_open()) {
        std::cout << "模拟文件系统不存在，是否初始化？(Y/N)\n";
        char a;
        std::cin >> a;
        if (a == 'Y' || a == 'y') {
            disk_init();
        } else {
            return 0;
        }
    }
    probe.close();
    read_info();

    while (curUser == -1) {
        int b = login(std::cin, std::cout);
        if (b == -1) {
            std::cout << "用户不存在或密码错误" << std::endl;
        } else {
            break;
        }
    }

    std::string order;
    while (true) {
        std::cout << get_path(curDir) << "/>";
        if (!(std::cin >> order)) {
            break;  // 输入结束（EOF）时退出
        }
        if (order == "EXIT" || order == "exit") {
            break;
        }
        process(std::cout, std::cin, order);
    }
    return 0;
}
