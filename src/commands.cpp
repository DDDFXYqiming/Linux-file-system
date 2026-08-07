#include "commands.h"
#include "filesystem.h"
#include "auth.h"

#include <fstream>
#include <iomanip>
#include <sstream>

void order_info(std::ostream& out) {
    out << " 系统信息\n";
    out << std::setw(15) << "  磁盘总空间： " << 100 * 1024 * 1024 << "B\n";
    out << std::setw(15) << "  已用空间： " << 100 * 1024 * 1024 - blockGroups[0].sBlock.all_freeBlocks * 1024 << " B\n";
    out << std::setw(15) << "  剩余空间： " << blockGroups[0].sBlock.all_freeBlocks * 1024 << " B\n";

    out << "\n 数据块组信息\n";
    out << std::setw(15) << "  数据块组数量 " << "100\n";
    out << std::setw(15) << "  每块组块数 " << "1024\n";

    out << "\n 磁盘块信息\n";
    out << std::setw(15) << "  块大小： " << "1024 B\n";
    out << std::setw(15) << "  块数量： " << "102400\n";
    out << std::setw(15) << "  已用磁盘块： " << 102400 - blockGroups[0].sBlock.all_freeBlocks << "\n";
    out << std::setw(15) << "  剩余磁盘块数量 " << blockGroups[0].sBlock.all_freeBlocks << "\n";
}

void order_cd(const std::string& pathName) {
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    if (!f_simdisk) {
        std::cout << "error_order_cd" << std::endl;
        return;
    }

    int diriNode = 0;
    Dir tempDir = curDir;
    std::string str;

    if (pathName == "..") {
        diriNode = iNode_table[curDir.iNode].father_iNode;
        f_simdisk.seekg(iNode_table[diriNode].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&curDir), sizeof(Dir));
        f_simdisk.close();
        return;
    }
    if (pathName == "/" || pathName == "root") {
        f_simdisk.seekg(iNode_table[0].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&curDir), sizeof(Dir));
        f_simdisk.close();
        return;
    }

    size_t pos = 0;
    if (pathName[0] == '/') {
        f_simdisk.seekg(iNode_table[0].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&tempDir), sizeof(Dir));
        pos = 1;
    } else if (pathName.rfind("root/", 0) == 0) {
        f_simdisk.seekg(iNode_table[0].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&tempDir), sizeof(Dir));
        pos = 5;
    }
    while (pos < pathName.size() && pathName.find('/', pos) != std::string::npos) {
        str = pathName.substr(pos, pathName.find('/', pos) - pos);
        if (str.empty()) {
            pos = pathName.find('/', pos) + 1;
            continue;
        }
        int n = tempDir.iNode_number;
        while (n) {
            if (iNode_table[tempDir.iNodes[n - 1]].Name == str) {
                diriNode = tempDir.iNodes[n - 1];
                break;
            }
            n--;
        }
        if (n == 0) {
            std::cout << "不存在的目录或文件：" << str << std::endl;
            f_simdisk.close();
            return;
        }
        f_simdisk.seekg(iNode_table[diriNode].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&tempDir), sizeof(Dir));
        pos = pathName.find('/', pos) + 1;
    }

    str = pathName.substr(pos);
    int n = tempDir.iNode_number;
    while (n) {
        if (iNode_table[tempDir.iNodes[n - 1]].Name == str) {
            diriNode = tempDir.iNodes[n - 1];
            break;
        }
        n--;
    }
    if (n == 0) {
        std::cout << "不存在的目录或文件：" << str << std::endl;
        f_simdisk.close();
        return;
    }

    f_simdisk.seekg(iNode_table[diriNode].BlockAddr, std::ios::beg);
    f_simdisk.read(reinterpret_cast<char*>(&curDir), sizeof(Dir));
    f_simdisk.close();
}

void order_md(const std::string& dirName, Dir& mdDir, std::ostream& out) {
    if (mdDir.iNode_number == MAX_SUBFILE) {
        out << "当前目录无法再创建子目录或文件\n";
        return;
    }
    if (blockGroups[0].sBlock.all_freeiNodes == 0) {
        out << "无可用 i节点\n";
        return;
    }
    if (blockGroups[0].sBlock.all_freeBlocks < DIR_SIZE) {
        out << "磁盘空间不足\n";
        return;
    }
    for (int i = 0; i < mdDir.iNode_number; i++) {
        if (dirName == iNode_table[mdDir.iNodes[i]].Name) {
            out << "文件或目录重名，创建失败" << std::endl;
            return;
        }
    }

    int newiNode = check_iNode();
    int newBlocks = check_blocks(DIR_SIZE);
    if (newBlocks == -1 || newiNode == -1) {
        out << "磁盘空间不足" << std::endl;
        return;
    }

    mdDir.iNodes[mdDir.iNode_number] = newiNode;
    mdDir.iNode_number++;

    Dir newDir;
    newDir.iNode = newiNode;
    newDir.iNode_number = 0;
    for (int i = 0, j = newBlocks; i < DIR_SIZE; i++, j++) {
        blocks_bitmap[j] = USED;
    }
    iNode_bitmap[newiNode] = USED;
    iNode_table[newiNode].BlockAddr = static_cast<int>(DATA_ADDR) + newBlocks * BLOCK_SIZE;
    iNode_table[newiNode].BlockNumber = DIR_SIZE;
    iNode_table[newiNode].fileType = DIRECTORY_TYPE;
    iNode_table[newiNode].Name = dirName;
    iNode_table[newiNode].size = static_cast<int>(sizeof(Dir));
    iNode_table[newiNode].father_iNode = mdDir.iNode;
    iNode_table[newiNode].uNumber = curUser;

    for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
        blockGroups[i].sBlock.all_freeBlocks -= DIR_SIZE;
        blockGroups[i].sBlock.all_freeiNodes -= 1;
    }

    save_all();
    f_simdisk.open("simdisk", std::ios::out | std::ios::binary | std::ios::in);
    f_simdisk.seekp(iNode_table[mdDir.iNode].BlockAddr, std::ios::beg);
    f_simdisk.write(reinterpret_cast<char*>(&mdDir), sizeof(Dir));
    f_simdisk.seekp(iNode_table[newDir.iNode].BlockAddr, std::ios::beg);
    f_simdisk.write(reinterpret_cast<char*>(&newDir), sizeof(Dir));
    f_simdisk.close();
    out << "创建成功" << std::endl;
}

void order_dir(std::ostream& out) {
    out << "文件或目录数量：" << curDir.iNode_number << std::endl;
    out << std::setw(15) << "文件类型" << std::setw(15) << "文件名" << std::setw(15) << "文件大小" << std::setw(15) << "文件属主" << std::endl;
    for (int i = 0; i < curDir.iNode_number; i++) {
        std::string filetype = iNode_table[curDir.iNodes[i]].fileType == DIRECTORY_TYPE ? "目录" : "文件";
        out << std::setw(15) << filetype
            << std::setw(15) << iNode_table[curDir.iNodes[i]].Name
            << std::setw(10) << iNode_table[curDir.iNodes[i]].size << " B"
            << std::setw(15) << users[iNode_table[curDir.iNodes[i]].uNumber].uName << std::endl;
    }
}

void order_rd(const std::string& pathName, std::ostream& out, std::istream& in) {
    int i = find_by_pathName(pathName);
    if (i <= 0) {
        return;
    }
    if (users[curUser].uType <= users[iNode_table[i].uNumber].uType &&
        users[curUser].uName != users[iNode_table[i].uNumber].uName) {
        out << "无操作权限" << std::endl;
        return;
    }

    if (iNode_table[i].fileType == DIRECTORY_TYPE) {
        Dir rdDir;
        f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
        f_simdisk.seekg(iNode_table[i].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&rdDir), sizeof(Dir));
        f_simdisk.close();
        if (rdDir.iNode_number > 0) {
            out << "目录非空，是否确认删除？(Y/N)" << std::endl;
            std::string a;
            in >> a;
            if (a == "Y" || a == "y") {
                delete_file(rdDir.iNode);
            } else {
                return;
            }
        } else {
            delete_file(rdDir.iNode);
        }
    } else {
        delete_file(i);
    }
    save_all();
    out << "删除成功" << std::endl;
}

void order_newfile(const std::string& filename, std::ostream& out, std::istream& in, const std::string& content) {
    std::string contentCopy = content;
    if (contentCopy.empty()) {
        out << "请输入文件内容（Ctrl+Z 结束）：" << std::endl;
        std::string b;
        char c;
        while (in >> b) {
            c = static_cast<char>(in.get());
            contentCopy += b;
            contentCopy += c;
        }
        in.clear();
    }
    int size = static_cast<int>(contentCopy.length());
    int blocks = (size / 1024) + 1;

    if (curDir.iNode_number == MAX_SUBFILE) {
        out << "当前目录无法再创建子目录或文件\n";
        return;
    }
    if (blockGroups[0].sBlock.all_freeiNodes == 0) {
        out << "无可用 i节点\n";
        return;
    }
    if (blockGroups[0].sBlock.all_freeBlocks < blocks) {
        out << "磁盘空间不足\n";
        return;
    }
    for (int i = 0; i < curDir.iNode_number; i++) {
        if (filename == iNode_table[curDir.iNodes[i]].Name) {
            out << "文件或目录重名，创建失败" << std::endl;
            return;
        }
    }

    int newiNode = check_iNode();
    int newBlocks = check_blocks(blocks);
    if (newBlocks == -1 || newiNode == -1) {
        out << "磁盘空间不足" << std::endl;
        return;
    }
    for (int i = 0, j = newBlocks; i < blocks; i++, j++) {
        blocks_bitmap[j] = USED;
    }
    curDir.iNodes[curDir.iNode_number] = newiNode;
    curDir.iNode_number += 1;

    iNode_bitmap[newiNode] = USED;
    iNode_table[newiNode].BlockAddr = static_cast<int>(DATA_ADDR) + newBlocks * BLOCK_SIZE;
    iNode_table[newiNode].BlockNumber = blocks;
    iNode_table[newiNode].fileType = FILE_TYPE;
    iNode_table[newiNode].Name = filename;
    iNode_table[newiNode].size = size;
    iNode_table[newiNode].father_iNode = curDir.iNode;
    iNode_table[newiNode].uNumber = curUser;

    for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
        blockGroups[i].sBlock.all_freeBlocks -= blocks;
        blockGroups[i].sBlock.all_freeiNodes -= 1;
    }

    save_all();
    f_simdisk.open("simdisk", std::ios::out | std::ios::binary | std::ios::in);
    f_simdisk.seekp(iNode_table[curDir.iNode].BlockAddr, std::ios::beg);
    f_simdisk.write(reinterpret_cast<char*>(&curDir), sizeof(Dir));
    f_simdisk.seekp(iNode_table[newiNode].BlockAddr, std::ios::beg);
    f_simdisk.write(contentCopy.c_str(), size);
    f_simdisk.close();
    out << "创建成功\n";
}

void order_cat(const std::string& pathName, std::ostream& out) {
    int iNode = find_by_pathName(pathName);
    if (iNode < 0) {
        return;
    }
    if (users[curUser].uType < users[iNode_table[iNode].uNumber].uType) {
        out << "无权访问" << std::endl;
        return;
    }

    std::string filename = iNode_table[iNode].Name;
    int f_iNode = iNode_table[iNode].father_iNode;
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, std::ios::beg);
    Dir f_Dir;
    f_simdisk.read(reinterpret_cast<char*>(&f_Dir), iNode_table[f_iNode].size);
    for (int i = 0; i < f_Dir.iNode_number; i++) {
        if (iNode_table[f_Dir.iNodes[i]].Name == filename &&
            iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE) {
            int fileSize = iNode_table[f_Dir.iNodes[i]].size;
            char* str = new char[fileSize + 1];
            str[fileSize] = '\0';
            f_simdisk.seekg(iNode_table[f_Dir.iNodes[i]].BlockAddr, std::ios::beg);
            f_simdisk.read(str, fileSize);
            f_simdisk.close();
            out << str << std::endl;
            delete[] str;
            return;
        }
    }
    f_simdisk.close();
}

void order_copy(const std::string& path1, const std::string& path2, std::ostream& out) {
    int iNode = find_by_pathName(path1);
    if (iNode < 0) {
        return;
    }
    std::string filename = iNode_table[iNode].Name;
    int f_iNode = iNode_table[iNode].father_iNode;
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, std::ios::beg);
    Dir f_Dir;
    f_simdisk.read(reinterpret_cast<char*>(&f_Dir), iNode_table[f_iNode].size);

    char* str1 = nullptr;
    int size = 0;
    int i = 0;
    for (i = 0; i < f_Dir.iNode_number; i++) {
        if (iNode_table[f_Dir.iNodes[i]].Name == filename &&
            iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE) {
            size = iNode_table[f_Dir.iNodes[i]].size;
            str1 = new char[size + 1];
            str1[size] = '\0';
            f_simdisk.seekg(iNode_table[f_Dir.iNodes[i]].BlockAddr, std::ios::beg);
            f_simdisk.read(str1, size);
            break;
        }
    }
    if (i == f_Dir.iNode_number) {
        out << "没有文件" << filename << std::endl;
        f_simdisk.close();
        return;
    }
    f_simdisk.close();

    f_iNode = find_by_pathName(path2);
    if (f_iNode < 0) {
        out << "目标目录不存在：" << path2 << std::endl;
        return;
    }
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, std::ios::beg);
    f_simdisk.read(reinterpret_cast<char*>(&f_Dir), iNode_table[f_iNode].size);

    int blocks = (size / 1024) + 1;
    if (f_Dir.iNode_number == MAX_SUBFILE) {
        out << "目录无法再创建子目录或文件\n";
        f_simdisk.close();
        return;
    }
    if (blockGroups[0].sBlock.all_freeiNodes == 0) {
        out << "无可用 i节点\n";
        f_simdisk.close();
        return;
    }
    if (blockGroups[0].sBlock.all_freeBlocks < blocks) {
        out << "磁盘空间不足\n";
        f_simdisk.close();
        return;
    }
    for (i = 0; i < f_Dir.iNode_number; i++) {
        if (filename == iNode_table[f_Dir.iNodes[i]].Name) {
            out << "文件或目录重名，创建失败" << std::endl;
            f_simdisk.close();
            return;
        }
    }

    int newiNode = check_iNode();
    int newBlocks = check_blocks(blocks);
    if (newBlocks == -1 || newiNode == -1) {
        out << "磁盘空间不足" << std::endl;
        f_simdisk.close();
        return;
    }
    for (int k = 0, j = newBlocks; k < blocks; k++, j++) {
        blocks_bitmap[j] = USED;
    }
    f_Dir.iNodes[f_Dir.iNode_number] = newiNode;
    f_Dir.iNode_number += 1;

    iNode_bitmap[newiNode] = USED;
    iNode_table[newiNode].BlockAddr = static_cast<int>(DATA_ADDR) + newBlocks * BLOCK_SIZE;
    iNode_table[newiNode].BlockNumber = blocks;
    iNode_table[newiNode].fileType = FILE_TYPE;
    iNode_table[newiNode].Name = filename;
    iNode_table[newiNode].size = size;
    iNode_table[newiNode].father_iNode = f_Dir.iNode;
    iNode_table[newiNode].uNumber = curUser;

    for (int k = 0; k < BLOCK_GROUP_NUMBER; k++) {
        blockGroups[k].sBlock.all_freeBlocks -= blocks;
        blockGroups[k].sBlock.all_freeiNodes -= 1;
    }
    f_simdisk.close();

    save_all();
    f_simdisk.open("simdisk", std::ios::out | std::ios::binary | std::ios::in);
    f_simdisk.seekp(iNode_table[f_Dir.iNode].BlockAddr, std::ios::beg);
    f_simdisk.write(reinterpret_cast<char*>(&f_Dir), sizeof(Dir));
    f_simdisk.seekp(iNode_table[newiNode].BlockAddr, std::ios::beg);
    f_simdisk.write(str1, size);
    f_simdisk.close();
    delete[] str1;
    out << "复制成功" << std::endl;
}

void order_copy_host_to_sim(const std::string& path1, const std::string& path2, std::ostream& out) {
    std::string h = host_path(path1);
    std::ifstream fin;
    fin.open(h, std::ios::in | std::ios::binary);
    if (!fin) {
        out << "无法打开宿主文件" << std::endl;
        return;
    }
    std::stringstream buf;
    buf << fin.rdbuf();
    std::string content = buf.str();
    fin.close();
    std::string filename = host_filename(h);

    int f_iNode = find_by_pathName(path2);
    if (f_iNode < 0) {
        out << "目标目录不存在：" << path2 << std::endl;
        return;
    }
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, std::ios::beg);
    Dir f_Dir;
    f_simdisk.read(reinterpret_cast<char*>(&f_Dir), iNode_table[f_iNode].size);

    int size = static_cast<int>(content.length());
    int blocks = (size / 1024) + 1;
    int i;
    if (f_Dir.iNode_number == MAX_SUBFILE) {
        out << "目录无法再创建子目录或文件\n";
        f_simdisk.close();
        return;
    }
    if (blockGroups[0].sBlock.all_freeiNodes == 0) {
        out << "无可用 i节点\n";
        f_simdisk.close();
        return;
    }
    if (blockGroups[0].sBlock.all_freeBlocks < blocks) {
        out << "磁盘空间不足\n";
        f_simdisk.close();
        return;
    }
    for (i = 0; i < f_Dir.iNode_number; i++) {
        if (filename == iNode_table[f_Dir.iNodes[i]].Name) {
            out << "文件或目录重名，创建失败" << std::endl;
            f_simdisk.close();
            return;
        }
    }

    int newiNode = check_iNode();
    int newBlocks = check_blocks(blocks);
    if (newBlocks == -1 || newiNode == -1) {
        out << "磁盘空间不足" << std::endl;
        f_simdisk.close();
        return;
    }
    for (int k = 0, j = newBlocks; k < blocks; k++, j++) {
        blocks_bitmap[j] = USED;
    }
    f_Dir.iNodes[f_Dir.iNode_number] = newiNode;
    f_Dir.iNode_number += 1;

    iNode_bitmap[newiNode] = USED;
    iNode_table[newiNode].BlockAddr = static_cast<int>(DATA_ADDR) + newBlocks * BLOCK_SIZE;
    iNode_table[newiNode].BlockNumber = blocks;
    iNode_table[newiNode].fileType = FILE_TYPE;
    iNode_table[newiNode].Name = filename;
    iNode_table[newiNode].size = size;
    iNode_table[newiNode].father_iNode = f_Dir.iNode;
    iNode_table[newiNode].uNumber = curUser;

    for (int k = 0; k < BLOCK_GROUP_NUMBER; k++) {
        blockGroups[k].sBlock.all_freeBlocks -= blocks;
        blockGroups[k].sBlock.all_freeiNodes -= 1;
    }
    f_simdisk.close();

    save_all();
    f_simdisk.open("simdisk", std::ios::out | std::ios::binary | std::ios::in);
    f_simdisk.seekp(iNode_table[f_Dir.iNode].BlockAddr, std::ios::beg);
    f_simdisk.write(reinterpret_cast<char*>(&f_Dir), sizeof(Dir));
    f_simdisk.seekp(iNode_table[newiNode].BlockAddr, std::ios::beg);
    f_simdisk.write(content.c_str(), size);
    f_simdisk.close();
    out << "复制成功" << std::endl;
}

void order_copy_sim_to_host(const std::string& path1, const std::string& path2, std::ostream& out) {
    int iNode = find_by_pathName(path1);
    if (iNode < 0) {
        return;
    }
    std::string filename = iNode_table[iNode].Name;
    int f_iNode = iNode_table[iNode].father_iNode;
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    f_simdisk.seekg(iNode_table[f_iNode].BlockAddr, std::ios::beg);
    Dir f_Dir;
    f_simdisk.read(reinterpret_cast<char*>(&f_Dir), iNode_table[f_iNode].size);

    char* str1 = nullptr;
    int size = 0;
    int i = 0;
    for (i = 0; i < f_Dir.iNode_number; i++) {
        if (iNode_table[f_Dir.iNodes[i]].Name == filename &&
            iNode_table[f_Dir.iNodes[i]].fileType == FILE_TYPE) {
            size = iNode_table[f_Dir.iNodes[i]].size;
            str1 = new char[size + 1];
            str1[size] = '\0';
            f_simdisk.seekg(iNode_table[f_Dir.iNodes[i]].BlockAddr, std::ios::beg);
            f_simdisk.read(str1, size);
            break;
        }
    }
    if (i == f_Dir.iNode_number) {
        out << "没有文件" << filename << std::endl;
        f_simdisk.close();
        return;
    }
    f_simdisk.close();

    std::string host_p = host_path(path2);
    std::string full = host_p + "\\" + filename;
    std::ofstream fout(full, std::ios::out | std::ios::binary);
    if (!fout) {
        // 目录形式不可用时，尝试把参数直接作为目标文件路径
        fout.open(host_p, std::ios::out | std::ios::binary);
        if (!fout) {
            out << "无法写入宿主文件：" << full << std::endl;
            delete[] str1;
            return;
        }
    }
    fout.write(str1, size);
    fout.close();
    delete[] str1;
    out << "复制成功" << std::endl;
}

void order_check() {
    save_all();
    std::cout << "磁盘修改成功" << std::endl;
}

void process(std::ostream& out, std::istream& in, const std::string& order) {
    if (order == "info") {
        order_info(out);
    } else if (order == "md") {
        std::string dirName;
        in >> dirName;
        order_md(dirName, curDir, out);
    } else if (order == "cd") {
        std::string pathName;
        in >> pathName;
        order_cd(pathName);
    } else if (order == "dir") {
        order_dir(out);
    } else if (order == "rd") {
        std::string pathName;
        in >> pathName;
        order_rd(pathName, out, in);
        read_info();
    } else if (order == "newfile") {
        std::string filename;
        in >> filename;
        std::string inlineContent;
        std::getline(in, inlineContent);
        if (!inlineContent.empty() && inlineContent[0] == ' ') {
            inlineContent.erase(0, 1);
        }
        if (inlineContent.empty()) {
            order_newfile(filename, out, in);  // 交互输入模式
        } else {
            order_newfile(filename, out, in, inlineContent);  // 命令行直接指定内容
        }
    } else if (order == "cat") {
        std::string pathName;
        in >> pathName;
        order_cat(pathName, out);
    } else if (order == "del") {
        std::string pathName;
        in >> pathName;
        order_rd(pathName, out, in);
        read_info();
    } else if (order == "copy") {
        std::string path1, path2, str1, str2;
        in >> path1 >> path2;
        str1 = path1.substr(0, 6);
        str2 = path2.substr(0, 6);
        if (str1 != "<host>" && str2 != "<host>") {
            order_copy(path1, path2, out);
        } else if (str1 == "<host>") {
            order_copy_host_to_sim(path1, path2, out);
        } else if (str2 == "<host>") {
            order_copy_sim_to_host(path1, path2, out);
        }
        read_info();
    } else if (order == "adduser") {
        std::string uName, password, uType;
        in >> uName >> password >> uType;
        if (users[curUser].uType == ROOT_USER) {
            add_user(uName, password, uType, out);
        } else {
            out << "无权操作" << std::endl;
        }
    } else if (order == "users") {
        order_users(out);
    } else if (order == "deluser") {
        std::string uName;
        in >> uName;
        if (users[curUser].uType == ROOT_USER) {
            delete_user(uName, out);
        } else {
            out << "无权操作" << std::endl;
        }
    } else if (order == "check") {
        order_check();
    } else {
        out << "无此命令" << std::endl;
    }
}
