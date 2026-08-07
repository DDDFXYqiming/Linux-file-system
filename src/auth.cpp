#include "auth.h"
#include "filesystem.h"

#include <iomanip>

int login(std::istream& in, std::ostream& out) {
    std::string u, p;
    out << "请输入用户名和密码：\n";
    in >> u >> p;
    for (int i = 0; i < MAX_USER; i++) {
        if (u == users[i].uName && p == users[i].password) {
            curUser = i;
            return 1;
        }
    }
    return -1;
}

void add_user(const std::string& uName, const std::string& password, const std::string& uType, std::ostream& out) {
    int u = -1;
    for (int i = 0; i < MAX_USER; i++) {
        if (users[i].uType == NOT_USED) {
            u = i;
            break;
        }
    }
    if (u == -1) {
        out << "用户数量已达上限" << std::endl;
        return;
    }
    for (int i = 0; i < MAX_USER; i++) {
        if (users[i].uName == uName) {
            out << "用户已存在" << std::endl;
            return;
        }
    }
    if (uType == "user" || uType == "USER") {
        users[u].uType = USER;
    } else if (uType == "SYSTEM" || uType == "system") {
        users[u].uType = SYSTEM_USER;
    } else if (uType == "ROOT" || uType == "root") {
        users[u].uType = ROOT_USER;
    } else {
        out << "不存在的用户类型" << std::endl;
        return;
    }
    users[u].uName = uName;
    users[u].password = password;
    save_all();
    out << "创建用户成功" << std::endl;
}

void delete_user(const std::string& uName, std::ostream& out) {
    if (uName == "root") {
        out << "不能删除 root 用户" << std::endl;
        return;
    }
    for (int i = 0; i < MAX_USER; i++) {
        if (users[i].uName == uName) {
            users[i].uType = NOT_USED;
            users[i].uName = "";
            users[i].password = "";
            save_all();
            out << "删除成功" << std::endl;
            return;
        }
    }
    out << "不存在的用户" << std::endl;
}

void order_users(std::ostream& out) {
    out << "用户信息：" << std::endl;
    out << std::setw(15) << "用户名" << std::setw(15) << "用户类型" << std::endl;
    for (int i = 0; i < MAX_USER; i++) {
        std::string type;
        if (users[i].uType == ROOT_USER) {
            type = "ROOT";
        } else if (users[i].uType == SYSTEM_USER) {
            type = "SYSTEM";
        } else if (users[i].uType == USER) {
            type = "USER";
        } else {
            continue;
        }
        out << std::setw(15) << users[i].uName << std::setw(15) << type << std::endl;
    }
}
