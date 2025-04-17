
# GuoShuBase数据库

## 项目介绍

实训课设

## PF组件说明

4.17编

PF 组件（Paged File Manager）是 **GuoShuBase** 的底层存储管理模块，它负责 **磁盘页面的分配、读取、写入和缓存**。它为更高级的组件（如记录管理 RM、索引管理 IX）提供 **页面级别的存储抽象**，并通过 **缓冲管理** 提高 I/O 性能。

**PF 组件的核心功能**
1. **文件管理（File Management）**
    - **创建、打开、关闭、删除分页文件**
    - 每个文件由 **多个固定大小的页面（Page）** 组成，每个页面都有唯一的 **PageNum**。

2. **页面管理（Page Management）**
    - **分配和释放页面**：支持动态申请和回收页面。
    - **读写页面**：允许加载磁盘上的页面到缓冲区并修改内容。

3. **缓冲区管理（Buffer Management）**
    - **缓存已访问的页面**，减少磁盘 I/O，提高性能。
    - 采用 **固定大小的缓冲池（Buffer Pool）**，实现 **页面替换策略**（如 LRU）。

4. **错误处理与返回码（Error Handling）**
    - 通过 `RC`（Return Code） 机制 **返回操作状态**，如：
        - `OK_RC = 0`（操作成功）
        - `START_PF_ERR ~ END_PF_ERR`（PF 组件错误）
        - `START_PF_WARN ~ END_PF_WARN`（PF 组件警告）

建议按如下顺序阅读代码（及代码中的注释）

- `guoshubase.h`
- `pf.h`
- `pf_internal.h`
- `pf_hashtable.h`
- `pf_hashtable.cpp`

**提示：**

> [!tip]
> However, a PF client can always send an explicit request to force (i.e., write to disk) the contents of a particular page, or to force all dirty pages of a file, without removing those pages from the buffer.
