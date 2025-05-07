
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


## RM组件说明

4.25编

- Record Manager（记录管理器）子系统向用户暴露RID（记录标识符），但隐藏了页面的底层细节，用户可以通过RM的高级抽象（特别是结合`FileScan`接口）操作记录。
- 代码按功能模块化组织：
   - 接口定义在`rm.h`中，包含多个类
   - 每个类有独立的实现文件（如`rm_filehandle.cpp`、`rm_record.cpp`）
   - 对应的Google Test单元测试文件（如`rm_filehandle_gtest.cpp`）
   - `rm_test.cpp`是基于原测试框架的扩展版本

---

### 核心数据结构

#### 页头 (RM_PageHdr)
- 使用**位图**跟踪页面内的空闲槽位
- 紧凑的数据结构设计以节省空间

#### 文件头 (RM_FileHdr)
- 采用**空闲链表**管理空闲页（类似PF组件）
- 特点：
   - 只需访问链表头部即可完成所有修改
   - 文件头独占一个完整页，确保记录页的对称性
   - 通过`FileHeader`中的页码跟踪第一个空闲页
   - 每个页头的`PageHeader`记录下一个空闲页

---

### 测试方案

1. **自动化单元测试**
   - 使用Google Test框架（类似JUnit风格）
   - 测试范围包括：
      - 跨打开/关闭操作的写入持久性
      - 不同记录大小的兼容性
      - 多页场景下的批量操作

2. **集成测试**
   - 扩展`pf_test*`和`rm_test`（增加扫描功能测试）
   - 测试框架自动验证结果，无需人工干预

---

### 已知问题/BUG

1. **代码标注**
   - 使用`//TODO`标记待改进项

2. **接口设计限制**
   - 强制返回错误码导致接口不够灵活
   - 建议改用异常处理机制（带简单跟踪）

3. **头文件管理**
   - 禁止`#include`导致`rm.h`过于臃肿（包含多个类声明）

4. **存储优化**
   - 当前磁盘记录冗余存储RID，可移除以节省I/O

---

### 关键问题解答

### 1. 计算每页记录槽数量的公式
**代码位置**: `rm_filehandle.cpp:211` (`RM_FileHandle::GetNumSlots`)  
**计算逻辑**：  
数据页由三部分组成：
1. 固定大小的页头静态部分
2. 位图占用的字节（每记录1 bit）
3. 记录数据

采用迭代计算：
- 初始假设每记录额外占用1/8字节（1 bit）
- 校验实际空间是否满足（需位图使用完整字节）

---

#### 2. RM_FileScan的状态维护
**代码位置**: `rm_filescan.cpp:133` (`RM_FileScan::GetNextRec`)  
**实现方案**：
- 在类成员`current`中保存最近返回的记录引用
- 优势：
   - 针对顺序访问优化（避免重复读取元数据/数据）
- 劣势：
   - 扫描过程中插入/更新记录会导致数据不一致

---

#### 3. 文件头缓存策略
**代码位置**: `rm.h:180` (`RM_FileHandle`类成员)  
**方案**：
- 在`RM_FileHandle`对象中缓存文件头副本
- 变更管理：
   - 通过`bHdrChanged`标志控制写回磁盘
- 假设条件：
   - 无并发修改（多句柄同时写会破坏缓存一致性）
   - 文件关闭后重新打开能正确同步数据

---

### 未通过测试案例

1. **记录更新验证失败**
   - 场景：添加记录→验证→更新→二次验证
   - 错误：检测到重复记录 `[a3999, 3999, 3999.000000]`

2. **扫描结果不符**
   - 场景：添加100条记录→扫描→验证
   - 错误：预期值50与实际扫描结果不匹配

3. **扫描中删除记录**
   - 场景：插入→开启扫描→删除→关闭扫描
   - 测试46失败（记录删除异常）

4. **空间复用校验**
   - 场景：删除→新增→扫描计数
   - 测试48失败（空间回收异常）

---

### 已修复问题

✅ **删除不存在的记录**
- 原问题：`DeleteRec`错误返回成功
- 修复：增加存在性校验

✅ **属性类型校验**
- 测试44：字符串长度超


## IX组件说明
5.7编

### B+树索引结构
- **索引条目组成**：键值 + 指向RM文件数据位置的RID
- **排序规则**：基于`<=`假设构建的B+树，中间节点的指针指向的子树包含该节点中的最大键值
- **节点布局**：
    - 中间节点和叶节点采用相同结构和大小
    - 中间节点虽不使用RID的SlotID部分，但保留结构以简化实现
    - 每个中间节点末尾存储一个（隐式）键值
    - 所有节点（含叶节点）均存储左右指针（PageID）
    - 叶节点严格保持有序，支持双向有序索引扫描

### 重复键处理
- 通过实现"最右匹配"保证方法，使所有操作都能感知重复键
- 相比桶页维护方案，该设计能获得更好的I/O性能

### 删除算法
- 采用惰性删除策略
- 当节点（中间/叶节点）剩余键数为0时触发下溢

### 扫描优化
- 主要通过扫描接口使用索引
- 使用对数级B+树查找优化扫描起止点，最小化扫描的叶节点数量
    - 示例：对不存在的键执行EQ_OP扫描时，首次调用GetNextEntry()即返回EOF
- 扫描类支持升序（默认）和降序模式，根据用户需求自动选择
- 通过当前叶节点/位置等状态信息优化扫描范围

---

### 核心数据结构

#### B+树节点
- 所有键值在页内保持有序，支持二分查找
- 无独立页头，但每个节点存储：
    - 左右PageID指针
    - 键值数量
    - 序列化信息

#### 文件头(RM_FileHdr)
- 记录根页面和B+树总页数
- 维护树高、阶数等全局信息
- 包含属性类型和长度等元数据

---

### 测试方案

#### 自动化测试
1. 使用类JUnit风格的GoogleTest框架进行单元测试
2. 修改版ix_test验证数据正确性
3. 多页面尺寸测试：覆盖插入/删除操作的典型用例

#### 测试内容
- 包含B+树不变式验证
- 确保每次操作后树保持平衡和正确性

---

### 已知问题
代码中标注`//TODO`的部分均为待解决问题

---

### 设计问答

#### 1. 索引容量计算
**问题**：假设索引整型属性且无重复值，计算索引扩展到4层前的最大条目数（不考虑桶页）。若每个值v对应2个rid会怎样？

**解答**：
- 节点容量计算见`BtreeNode::BtreeNode()` (btree_node.cc:15)
    - 依赖属性大小和RID尺寸
    - 需考虑左右指针和键数占用空间
    - 隐式最右条目计入统计
- 测试案例显示：PF_PAGE_SIZE(4092)下每节点可存340个条目 (btree_node_gtest.cc:73)
- 层级容量：
    - 第1层：340条目
    - 第2层：340×340条目
    - 第3层：340³条目
    - 超过340³将触发第4层
- 注：340=指针数，键数=339（显式）+1（隐式）

#### 2. 删除算法
**实现方案**：
- 采用惰性删除的迭代算法 (见`IndexHandle::DeleteEntry()` ix_indexhandle.cc:220)
    - 从叶节点开始向上删除
    - 仅当节点条目全空时执行删除
- 递归索引收缩：
    - 迭代检查每层是否发生"下溢"
    - 根据下溢情况调整父节点指针

#### 3. RID存储方案
**当前设计**：
- 直接存储在叶节点中 (见`BtreeNode::BtreeNode()` btree_node.cc:35)
    - **优势**：
        - 无需单独管理桶页
        - 避免扫描时打开桶页的开销
        - 规避固定桶大小导致的存储效率问题
    - **代价**：
        - 需实现更复杂的重复键扫描逻辑（如<=、>=操作）

**改进方向**：
若时间允许，将优化叶节点和RID的存储结构

### 一个测试问题的记录-ix组件未能正确释放pin页面问题

这个问题本质上还未能解决，目前选择通过在读取未成功释放的页面时直接返回对应指针的方法，即：

```cpp
if (!(rc = hashTable.Find(fd, pageNum, slot))) {
    *ppBuffer = bufTable[slot].pData; // 直接返回指针
    return 0;
}
```

可能导致问题：
- **竞态条件**：如果两个线程同时请求同一页面，可能同时获取到同一缓冲区的指针，导致数据竞争。
- **脏读风险**：若页面正在被另一个事务修改，当前事务可能读到未提交的数据。
- 直接复用页面 **未增加 pin count**，可能导致：
    - 页面被缓冲管理器意外置换（因 pin count=0）。
    - 真正的使用者调用 `UnpinPage` 时错误减少 pin count。
- 在 `READ COMMITTED` 或 `SERIALIZABLE` 隔离级别下，直接复用缓冲区可能违反隔离性。

分析原因：
1. **页面未被正确标记为未使用**
- 原 `FlushPages` 在遇到 pinned 页面时直接跳过：
  ```cpp
  if (bufTable[slot].pinCount) {
      rcWarn = PF_PAGEPINNED;
      continue; // 跳过 pinned 页面
  }
  ```
  导致这些页面 **长期滞留** 在缓冲区中。

2. **Pin/Unpin 调用不匹配**
- 可能存在的 bug：
    - **漏调 `UnpinPage`**：某些代码路径忘记调用。
    - **重复 Pin**：同一页面被多次 Pin 但只 Unpin 一次。
    - **提前 Unpin**：在页面仍需使用时误调 Unpin。

3. **哈希表状态不一致**
- 若 `hashTable.Delete` 未正确执行（如因 pinned 页面跳过），后续 `AllocatePage` 会误判页面仍存在。

---

#### 可能修改方案
1. **修改 `AllocatePage`：强制 Pin 计数**
```cpp
if (!(rc = hashTable.Find(fd, pageNum, slot))) {
    // 页面已存在时，必须增加 pin count
    bufTable[slot].pinCount++; 
    *ppBuffer = bufTable[slot].pData;
#ifdef PF_LOG
    sprintf(psMessage, "Reusing pinned page (%d,%d) (new pin count=%d).\n", 
            fd, pageNum, bufTable[slot].pinCount);
    WriteLog(psMessage);
#endif
    return 0;
}
```

- 显式增加 pin count，避免缓冲区被意外回收。
- 调用者仍需负责调用 `UnpinPage`，符合原有设计。

2. **增强 `FlushPages`：记录滞留页面**
3. 
```cpp
if (bufTable[slot].pinCount) {
    rcWarn = PF_PAGEPINNED;
#ifdef PF_LOG
    sprintf(psMessage, "PAGE LEAK: File=%d Page=%d Slot=%d PinCount=%d\n",
            fd, bufTable[slot].pageNum, slot, bufTable[slot].pinCount);
    WriteLog(psMessage);
#endif
    slot = next;
    continue;
}
```

3. **添加校验函数（调试用）**
```cpp
void PF_BufferMgr::ValidatePins() const {
    int slot = first;
    while (slot != INVALID_SLOT) {
        if (bufTable[slot].pinCount < 0) {
            cerr << "ERROR: Slot " << slot << " has negative pin count!" << endl;
        }
        slot = bufTable[slot].next;
    }
}
```

由于工时原因，目前仍采用复用的方法，还未见其导致的明显错误。


