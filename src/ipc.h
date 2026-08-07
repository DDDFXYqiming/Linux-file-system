#pragma once

#include <cstddef>
#include <string>

// 跨平台共享内存通道（Windows CreateFileMapping / POSIX shm_open+mmap）
class SharedMemory {
public:
    SharedMemory() = default;
    ~SharedMemory();
    // 不存在则创建，存在则打开；失败返回 false
    bool openOrCreate(const std::string& name, std::size_t size);
    void* data() const { return mapped_; }
    std::size_t size() const { return size_; }
    void close();

private:
    void* mapped_ = nullptr;
    std::size_t size_ = 0;
#ifdef _WIN32
    void* handle_ = nullptr;
#else
    int fd_ = -1;
#endif
};

// 跨进程消息（定长字符数组，避免 std::string 跨进程失效）
struct IpcMessage {
    int flag = 0;          // 0=空闲 1=命令/登录 2=路径提示 3=输出
    int uNumber = -1;      // 写者用户序号
    char order[64] = {0};  // 命令名
    char data1[512] = {0}; // 参数1（登录用户名 / 命令参数1）
    char data2[512] = {0}; // 参数2（登录密码 / 命令参数2）
    char data3[2048] = {0}; // 输出内容 / 路径提示
};
