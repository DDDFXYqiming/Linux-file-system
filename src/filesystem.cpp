#include "filesystem.h"

#include <cstring>
#include <iostream>

int iNode_bitmap[INODE_NUMBER];
i_Node iNode_table[INODE_NUMBER];
int blocks_bitmap[BLOCKS];
BlockGroup blockGroups[BLOCK_GROUP_NUMBER];

Dir curDir;
int curUser = -1;
std::fstream f_simdisk;
User users[MAX_USER];

void disk_init() {
    for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
        blockGroups[i].sBlock.all_freeBlocks = BLOCKS - DIR_SIZE;
        blockGroups[i].sBlock.all_freeiNodes = INODE_NUMBER - 1;
        blockGroups[i].BlockAddr = static_cast<int>(DATA_ADDR) + i * BLOCK_SIZE * BLOCK_NUMBER;
        blockGroups[i].BlockMap = i * BLOCK_NUMBER;
        blockGroups[i].freeBlocks = BLOCK_NUMBER;
        blockGroups[i].freeiNode = BLOCK_NUMBER;
        blockGroups[i].iNodeMap = i * BLOCK_NUMBER;
        blockGroups[i].iNodeTable = i * BLOCK_NUMBER;
    }
    for (int i = 0; i < INODE_NUMBER; i++) {
        iNode_bitmap[i] = NOT_USED;
        blocks_bitmap[i] = NOT_USED;
        iNode_table[i].BlockAddr = 0;
        iNode_table[i].BlockNumber = 0;
        iNode_table[i].fileType = -1;
        iNode_table[i].Name = "new file";
        iNode_table[i].size = 0;
        iNode_table[i].father_iNode = -1;
    }
    for (int i = 0; i < MAX_USER; i++) {
        users[i].uName = "";
        users[i].password = "";
        users[i].uType = NOT_USED;
    }
    users[0].uName = "root";
    users[0].password = "password";
    users[0].uType = ROOT_USER;

    curDir.iNode = 0;
    curDir.iNode_number = 0;

    blockGroups[0].freeBlocks -= DIR_SIZE;
    blockGroups[0].freeiNode -= 1;
    for (int i = 0; i < DIR_SIZE; i++) {
        blocks_bitmap[i] = USED;
    }
    iNode_bitmap[0] = USED;
    iNode_table[0].BlockAddr = static_cast<int>(DATA_ADDR);
    iNode_table[0].BlockNumber = DIR_SIZE;
    iNode_table[0].fileType = DIRECTORY_TYPE;
    iNode_table[0].Name = "root";
    iNode_table[0].size = static_cast<int>(sizeof(Dir));
    iNode_table[0].father_iNode = 0;
    iNode_table[0].uNumber = 0;

    f_simdisk.open("simdisk", std::ios::out | std::ios::binary);
    if (!f_simdisk) {
        std::cout << "error_disk_init" << std::endl;
        return;
    }
    for (int i = 0; i < MAX_USER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&users[i]), sizeof(User));
    }
    for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&blockGroups[i]), sizeof(BlockGroup));
    }
    for (int i = 0; i < INODE_NUMBER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&iNode_bitmap[i]), sizeof(int));
    }
    for (int i = 0; i < INODE_NUMBER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&iNode_table[i]), sizeof(i_Node));
    }
    for (int i = 0; i < BLOCKS; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&blocks_bitmap[i]), sizeof(int));
    }
    // 预留 100MB 数据空间（先写，避免覆盖 DATA_ADDR 处的目录块）
    const long long dataSize = static_cast<long long>(BLOCKS) * BLOCK_SIZE;
    char* buffer = new char[dataSize];
    std::memset(buffer, 0, dataSize);
    f_simdisk.write(buffer, dataSize);
    delete[] buffer;
    // 再写根目录到 DATA_ADDR
    f_simdisk.seekp(iNode_table[curDir.iNode].BlockAddr, std::ios::beg);
    f_simdisk.write(reinterpret_cast<char*>(&curDir), sizeof(Dir));
    f_simdisk.close();
}

void read_info() {
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    if (!f_simdisk) {
        std::cout << "error_read_info" << std::endl;
        return;
    }
    for (int i = 0; i < MAX_USER; i++) {
        f_simdisk.read(reinterpret_cast<char*>(&users[i]), sizeof(User));
    }
    for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
        f_simdisk.read(reinterpret_cast<char*>(&blockGroups[i]), sizeof(BlockGroup));
    }
    for (int i = 0; i < INODE_NUMBER; i++) {
        f_simdisk.read(reinterpret_cast<char*>(&iNode_bitmap[i]), sizeof(int));
    }
    for (int i = 0; i < INODE_NUMBER; i++) {
        f_simdisk.read(reinterpret_cast<char*>(&iNode_table[i]), sizeof(i_Node));
    }
    for (int i = 0; i < BLOCKS; i++) {
        f_simdisk.read(reinterpret_cast<char*>(&blocks_bitmap[i]), sizeof(int));
    }
    if (curDir.iNode >= 0 && curDir.iNode < INODE_NUMBER) {
        f_simdisk.seekg(iNode_table[curDir.iNode].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&curDir), sizeof(Dir));
    }
    f_simdisk.close();
}

void save_all() {
    f_simdisk.open("simdisk", std::ios::out | std::ios::binary | std::ios::in);
    if (!f_simdisk) {
        std::cout << "error_save_all" << std::endl;
        return;
    }
    for (int i = 0; i < MAX_USER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&users[i]), sizeof(User));
    }
    for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&blockGroups[i]), sizeof(BlockGroup));
    }
    for (int i = 0; i < INODE_NUMBER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&iNode_bitmap[i]), sizeof(int));
    }
    for (int i = 0; i < INODE_NUMBER; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&iNode_table[i]), sizeof(i_Node));
    }
    for (int i = 0; i < BLOCKS; i++) {
        f_simdisk.write(reinterpret_cast<char*>(&blocks_bitmap[i]), sizeof(int));
    }
    f_simdisk.close();
}

std::string get_path(const Dir& dir) {
    std::string path, curName, temp;
    int iNode = dir.iNode;
    while (iNode) {
        curName = "/" + iNode_table[iNode].Name;
        temp = path;
        path = curName + temp;
        iNode = iNode_table[iNode].father_iNode;
    }
    temp = path;
    path = "root" + temp;
    return path;
}

int check_iNode() {
    for (int i = 0; i < INODE_NUMBER; i++) {
        if (iNode_bitmap[i] == NOT_USED) {
            return i;
        }
    }
    return -1;
}

int check_blocks(int size) {
    int j = 0;
    for (int i = 0; i < BLOCKS; i++) {
        if (blocks_bitmap[i] == NOT_USED) {
            j++;
            if (j == size) {
                return i + 1 - j;
            }
        } else {
            j = 0;
        }
    }
    return -1;
}

int find_by_pathName(const std::string& pathName) {
    if (pathName == "/" || pathName == "root") {
        return 0;
    }
    f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
    if (!f_simdisk) {
        std::cout << "error_find_by_pathname" << std::endl;
        return -1;
    }

    size_t pos = 0;
    Dir tempDir = curDir;
    // 绝对路径：从根目录开始解析
    if (pathName[0] == '/') {
        f_simdisk.seekg(iNode_table[0].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&tempDir), sizeof(Dir));
        pos = 1;
    } else if (pathName.rfind("root/", 0) == 0) {
        f_simdisk.seekg(iNode_table[0].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&tempDir), sizeof(Dir));
        pos = 5;
    }
    int diriNode = 0;
    std::string str;

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
            return -1;
        }
        f_simdisk.seekg(iNode_table[diriNode].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&tempDir), sizeof(Dir));
        pos = pathName.find('/', pos) + 1;
    }

    str = pathName.substr(pos);
    if (str.empty()) {
        f_simdisk.close();
        return tempDir.iNode;
    }
    int n = tempDir.iNode_number;
    while (n) {
        if (iNode_table[tempDir.iNodes[n - 1]].Name == str) {
            diriNode = tempDir.iNodes[n - 1];
            break;
        }
        n--;
    }
    f_simdisk.close();
    if (n == 0) {
        std::cout << "不存在的目录或文件：" << str << std::endl;
        return -1;
    }
    return diriNode;
}

int addr_to_bitmap(long long addr) {
    return static_cast<int>((addr - DATA_ADDR) / BLOCK_SIZE);
}

void delete_file(int iNode) {
    if (iNode_table[iNode].fileType == FILE_TYPE) {
        int bitmap = addr_to_bitmap(iNode_table[iNode].BlockAddr);
        for (int i = 0; i < iNode_table[iNode].BlockNumber; i++, bitmap++) {
            blocks_bitmap[bitmap] = NOT_USED;
        }
        for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
            blockGroups[i].sBlock.all_freeBlocks += iNode_table[iNode].BlockNumber;
            blockGroups[i].sBlock.all_freeiNodes += 1;
        }
        iNode_bitmap[iNode] = NOT_USED;

        Dir f_Dir;
        f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
        f_simdisk.seekg(iNode_table[iNode_table[iNode].father_iNode].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&f_Dir), sizeof(Dir));
        f_simdisk.close();
        for (int i = 0; i < f_Dir.iNode_number; i++) {
            if (iNode_table[f_Dir.iNodes[i]].Name == iNode_table[iNode].Name) {
                f_Dir.iNodes[i] = f_Dir.iNodes[f_Dir.iNode_number - 1];
                break;
            }
        }
        f_Dir.iNode_number -= 1;
        f_simdisk.open("simdisk", std::ios::out | std::ios::binary | std::ios::in);
        f_simdisk.seekp(iNode_table[iNode_table[iNode].father_iNode].BlockAddr, std::ios::beg);
        f_simdisk.write(reinterpret_cast<char*>(&f_Dir), sizeof(Dir));
        f_simdisk.close();
    } else {
        Dir rdDir;
        f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
        f_simdisk.seekg(iNode_table[iNode].BlockAddr, std::ios::beg);
        f_simdisk.read(reinterpret_cast<char*>(&rdDir), sizeof(Dir));
        f_simdisk.close();

        if (rdDir.iNode_number == 0) {
            int bitmap = addr_to_bitmap(iNode_table[rdDir.iNode].BlockAddr);
            for (int i = 0; i < iNode_table[rdDir.iNode].BlockNumber; i++, bitmap++) {
                blocks_bitmap[bitmap] = NOT_USED;
            }
            for (int i = 0; i < BLOCK_GROUP_NUMBER; i++) {
                blockGroups[i].sBlock.all_freeBlocks += iNode_table[rdDir.iNode].BlockNumber;
                blockGroups[i].sBlock.all_freeiNodes += 1;
            }
            iNode_bitmap[rdDir.iNode] = NOT_USED;

            Dir f_Dir;
            f_simdisk.open("simdisk", std::ios::in | std::ios::binary);
            f_simdisk.seekg(iNode_table[iNode_table[rdDir.iNode].father_iNode].BlockAddr, std::ios::beg);
            f_simdisk.read(reinterpret_cast<char*>(&f_Dir), sizeof(Dir));
            f_simdisk.close();
            for (int i = 0; i < f_Dir.iNode_number; i++) {
                if (iNode_table[f_Dir.iNodes[i]].Name == iNode_table[rdDir.iNode].Name) {
                    f_Dir.iNodes[i] = f_Dir.iNodes[f_Dir.iNode_number - 1];
                    break;
                }
            }
            f_Dir.iNode_number -= 1;
            f_simdisk.open("simdisk", std::ios::out | std::ios::binary | std::ios::in);
            f_simdisk.seekp(iNode_table[iNode_table[rdDir.iNode].father_iNode].BlockAddr, std::ios::beg);
            f_simdisk.write(reinterpret_cast<char*>(&f_Dir), sizeof(Dir));
            f_simdisk.close();
        } else {
            int n = rdDir.iNode_number;
            for (int i = 0; i < n; i++) {
                delete_file(rdDir.iNodes[i]);
            }
            delete_file(rdDir.iNode);
        }
    }
}

std::string host_path(const std::string& pathName) {
    std::string str = pathName.substr(6);
    if (str.size() <= 2 || str[2] != '\\') {
        if (str.size() >= 2) {
            return str.substr(0, 2) + "\\" + str.substr(2);
        }
        return str;
    }
    return str;
}

std::string host_filename(const std::string& pathName) {
    size_t pos = pathName.rfind('\\');
    if (pos == std::string::npos) {
        pos = pathName.rfind('/');
    }
    if (pos == std::string::npos) {
        return pathName;
    }
    return pathName.substr(pos + 1);
}
