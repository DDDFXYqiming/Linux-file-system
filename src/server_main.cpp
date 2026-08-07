#include "commands.h"
#include "filesystem.h"
#include "ipc.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP_MS(x) usleep((x) * 1000)
#endif

// 后台内核模式：通过共享内存接收 shell 客户端指令
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

    SharedMemory shm;
    if (!shm.openOrCreate("ShareMemorySZHC", sizeof(IpcMessage))) {
        std::cout << "共享内存创建失败" << std::endl;
        return 1;
    }
    std::memset(shm.data(), 0, sizeof(IpcMessage));
    auto* msg = static_cast<IpcMessage*>(shm.data());
    std::cout << "simdisk-server 已启动，等待 shell 客户端连接..." << std::endl;

    while (true) {
        SLEEP_MS(100);
        if (msg->flag == 1 && curUser == -1) {
            // 登录请求
            std::string u(msg->data1);
            std::string p(msg->data2);
            int found = -1;
            for (int i = 0; i < MAX_USER; i++) {
                if (u == users[i].uName && p == users[i].password) {
                    found = i;
                    break;
                }
            }
            if (found >= 0) {
                curUser = found;
                std::string path = get_path(curDir);
                std::cout << "用户登录成功：" << u << std::endl;
                msg->flag = 2;
                std::snprintf(msg->data3, sizeof(msg->data3), "%s/>", path.c_str());
            } else {
                std::cout << "登录失败：" << u << std::endl;
                msg->flag = 0;
                std::snprintf(msg->data3, sizeof(msg->data3), "用户名或密码错误");
            }
        } else if (msg->flag == 1 && curUser >= 0) {
            // 命令请求
            std::string order(msg->order);
            std::string data1(msg->data1);
            std::string data2(msg->data2);
            std::ostringstream oss;
            if (order == "cd") {
                order_cd(data1);
            } else if (order == "md") {
                order_md(data1, curDir, oss);
            } else if (order == "rd" || order == "del") {
                order_rd(data1, oss, std::cin);
                read_info();
            } else if (order == "newfile") {
                order_newfile(data1, oss, std::cin, data2);
            } else if (order == "cat") {
                order_cat(data1, oss);
            } else if (order == "copy") {
                std::string s1 = data1.substr(0, 6);
                std::string s2 = data2.substr(0, 6);
                if (s1 != "<host>" && s2 != "<host>") {
                    order_copy(data1, data2, oss);
                } else if (s1 == "<host>") {
                    order_copy_host_to_sim(data1, data2, oss);
                } else if (s2 == "<host>") {
                    order_copy_sim_to_host(data1, data2, oss);
                }
                read_info();
            } else if (order == "info") {
                order_info(oss);
            } else if (order == "dir") {
                order_dir(oss);
            } else {
                oss << "无此命令" << std::endl;
            }
            std::cout << "执行命令：" << order << " " << data1 << " " << data2 << std::endl;
            msg->flag = 3;
            std::snprintf(msg->data3, sizeof(msg->data3), "%s", oss.str().c_str());
            // 输出显示后回到路径提示状态
            SLEEP_MS(100);
            msg->flag = 2;
            std::snprintf(msg->data3, sizeof(msg->data3), "%s/>", get_path(curDir).c_str());
        }
    }
    return 0;
}
