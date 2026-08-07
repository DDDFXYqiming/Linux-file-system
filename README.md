# Linux 文件系统模拟器（simdisk）

课程设计：用 C++ 模拟 Linux 文件系统，磁盘空间 100MB，支持用户管理、文件读写保护、目录操作与宿主机文件互拷。

原版为 VS2017 + Windows API 工程（注释 GBK 乱码、强绑 Windows、单文件组织），本仓库 `refactor` 分支完成现代化重构：

- **CMake 跨平台构建**（Windows / Linux / macOS，C++17）
- 源码统一 **UTF-8 编码**（原 GBK 注释乱码）
- 按职责拆分：`filesystem`（磁盘核心）/ `commands`（命令集）/ `auth`（用户管理）/ `ipc`（共享内存抽象）
- 共享内存跨平台实现（Windows `CreateFileMapping` / POSIX `shm_open+mmap`），协议改为定长字符数组（原版把 `std::string` 直接 memcpy 进共享内存，跨进程必然失效）
- 修复多个原版缺陷：初始化布局覆盖根目录、`delete[]` 泄漏、`fstream` 打开模式错误、导出文件无错误检查、EOF 死循环等

## 构建

```bash
cmake -S . -B build
cmake --build build
```

生成三个可执行文件：
- `simdisk`：控制台模式（推荐，功能完整）
- `simdisk-server`：后台内核模式（通过共享内存接收指令）
- `simdisk-shell`：shell 客户端（与 server 通信）

## 使用（控制台模式）

```bash
cd build
./simdisk
```

首次运行输入 `Y` 初始化 100MB 模拟磁盘，然后登录（默认 `root` / `password`）。

## 命令

| 命令 | 说明 |
| --- | --- |
| `info` | 显示磁盘系统信息 |
| `cd <路径>` | 切换目录，支持 `/`、`..`、相对/绝对路径 |
| `dir` | 显示当前目录（类型/名称/大小/属主） |
| `md <目录名>` | 创建目录 |
| `rd <路径>` | 删除目录（非空时确认，递归删除） |
| `newfile <文件名> [内容]` | 创建文件；不带内容时进入交互输入（Ctrl+Z 结束） |
| `cat <路径>` | 显示文件内容（校验权限） |
| `copy <源> <目标>` | 模拟文件系统内部复制 |
| `copy <host>盘:\路径 <目标目录>` | 宿主文件拷入模拟盘 |
| `copy <模拟路径> <host>盘:\目录` | 模拟盘文件导出到宿主 |
| `del <路径>` | 删除文件 |
| `adduser <用户名> <密码> <user/system/root>` | root 添加用户 |
| `deluser <用户名>` | root 删除用户 |
| `users` | 用户列表 |
| `check` | 将内存状态写回磁盘 |
| `exit` | 退出 |

## 双进程模式（实验性）

```bash
# 终端 1：启动内核
./simdisk-server
# 终端 2：启动 shell 客户端
./simdisk-shell
```

说明：该模式原版基于 Windows 共享内存 + `std::string` 直接跨进程拷贝（不可用），重构后协议改为定长字符数组并在 Windows/POSIX 双实现；共享内存跨进程通信在部分受限/后台环境下可能存在权限隔离，日常使用推荐控制台模式。
