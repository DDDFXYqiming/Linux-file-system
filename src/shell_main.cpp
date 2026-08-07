#include "ipc.h"

#include <cstring>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP_MS(x) usleep((x) * 1000)
#endif

// Shell 客户端：与 simdisk-server 通过共享内存通信
int main() {
    SharedMemory shm;
    if (!shm.openOrCreate("ShareMemorySZHC", sizeof(IpcMessage))) {
        std::cout << "无法连接共享内存，请先启动 simdisk-server" << std::endl;
        return 1;
    }
    auto* msg = static_cast<IpcMessage*>(shm.data());

    // 登录
    while (true) {
        std::string u, p;
        std::cout << "用户名: ";
        std::cin >> u;
        std::cout << "密码: ";
        std::cin >> p;
        msg->flag = 1;
        msg->uNumber = -1;
        std::snprintf(msg->data1, sizeof(msg->data1), "%s", u.c_str());
        std::snprintf(msg->data2, sizeof(msg->data2), "%s", p.c_str());
        std::snprintf(msg->data3, sizeof(msg->data3), "%s", "");
        bool done = false;
        for (int i = 0; i < 100 && !done; i++) {
            SLEEP_MS(100);
            if (msg->flag == 2) {
                done = true;  // 登录成功，进入命令循环
            } else if (msg->flag == 0) {
                std::cout << "登录失败: " << msg->data3 << std::endl;
                done = true;
            }
        }
        if (msg->flag == 2) {
            break;
        }
        if (!done) {
            std::cout << "服务器无响应" << std::endl;
            return 1;
        }
    }

    // 命令循环
    std::cin.ignore();  // 清掉登录输入残留的换行
    while (true) {
        // 等待路径提示
        while (msg->flag != 2) {
            SLEEP_MS(100);
        }
        std::cout << msg->data3;
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;  // 输入结束（EOF）时退出
        }
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string order;
        iss >> order;
        if (order == "exit" || order == "EXIT") {
            break;
        }
        std::string a1, a2;
        iss >> a1 >> a2;
        *msg = IpcMessage{};
        msg->flag = 1;
        msg->uNumber = -1;
        std::snprintf(msg->order, sizeof(msg->order), "%s", order.c_str());
        std::snprintf(msg->data1, sizeof(msg->data1), "%s", a1.c_str());
        std::snprintf(msg->data2, sizeof(msg->data2), "%s", a2.c_str());
        // 等待输出
        while (msg->flag != 3) {
            SLEEP_MS(100);
        }
        std::cout << msg->data3 << std::endl;
    }
    return 0;
}
