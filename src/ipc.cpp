#include "ipc.h"

#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

SharedMemory::~SharedMemory() {
    close();
}

bool SharedMemory::openOrCreate(const std::string& name, std::size_t size) {
    close();
    size_ = size;
#ifdef _WIN32
    handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    if (handle_ == nullptr) {
        handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                     static_cast<DWORD>(size), name.c_str());
        if (handle_ == nullptr) {
            return false;
        }
    }
    mapped_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size);
    return mapped_ != nullptr;
#else
    fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd_ < 0) {
        return false;
    }
    if (ftruncate(fd_, static_cast<off_t>(size)) != 0) {
        close();
        return false;
    }
    mapped_ = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    return mapped_ != MAP_FAILED;
#endif
}

void SharedMemory::close() {
    if (mapped_ != nullptr) {
#ifdef _WIN32
        UnmapViewOfFile(mapped_);
#else
        munmap(mapped_, size_);
#endif
        mapped_ = nullptr;
    }
#ifdef _WIN32
    if (handle_ != nullptr) {
        CloseHandle(static_cast<HANDLE>(handle_));
        handle_ = nullptr;
    }
#else
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
#endif
}
