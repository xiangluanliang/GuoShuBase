
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


## SM组件说明

### 总体设计

该模块实现了将系统目录（system catalogs）作为数据库中的关系（relations）进行管理的方式，并提供了初始化和与数据库中表/索引交互所需的命令。

- `relcat` 和 `attrcat` 被构建为具有固定模式的 RM 文件。
- 创建了一个名为 `DataRelInfo` 的结构体（类似于 `DataAttrInfo`），用于表示 `relcat` 中的每条记录。
- `attrcat` 中的每条记录实质上是一个 `DataAttrInfo` 结构体。

---

### 关键数据结构

`relcat` 中维护的信息包括：

- `recordSize`：每条记录的大小
- `attrCount`：属性数量
- `numPages`：该关系使用的页数
- `numRecords`：该关系中的记录数
- `relName[MAXNAME+1]`：关系名称

目前，`numPages` 和 `numRecords` 由 `SM_Manager::Load()` 填充，但其他 DML 操作也需要维护这些字段，以保证其作为系统统计信息的有效性。

---

### 测试

- 使用自动化单元测试对各个类进行了测试。
- 使用了一种流行的测试框架 Google Test，其风格类似 JUnit，使测试过程快速自动化。
- 参见 `sm_manager_gtest.cc`。
- 同时生成了测试用的数据文件，配合建议的 `stars/soaps` 数据进行测试。

---

### 已知 Bug / 问题

希望能像操作关系一样打印索引内容。我曾考虑过通过在 `relcat/attrcat` 中添加条目的方式将索引作为完整关系暴露出来，但最终放弃了此方案，以避免妨碍用户创建带有 `.number` 后缀的表。

---

### 1 (a) 系统目录访问频繁，因此建议在打开数据库时也打开系统目录，并在数据库关闭时才关闭目录。你是否实现了该机制？为什么？

是的，我实现了该机制。这样做是因为我认为保持目录文件的打开句柄在代码中是一种方便且有效的做法，可以避免每次执行查询时都需要额外的 IO 或文件系统操作来重新打开目录。

参见 `SM_Manager::OpenDB()`（位于 `sm_manager.cc:50`）来查看如何初始化目录句柄。之后，代码中直接使用这些数据成员访问目录，无需每次重新打开。

---

### 1 (b) 如果你实现了上面的机制，那么对系统目录的更新可能不会立即写入磁盘。例如，若第二次打开目录（如为了实现 help 工具或打印目录内容），可能看不到目录的最新状态。你是如何处理这一问题的？

我通过拦截对目录的所有访问来处理这个问题。因为是单用户、单访问系统，所有调用都通过同一个 `SM_Manager` 实例进行。该实例始终保持目录文件的打开句柄，并在类内部拦截目录访问调用，直接通过成员对象提供目录内容。

参见 `SM_Manager::Print()`（位于 `sm_manager.cc:599`）查看拦截调用的实现方式。

---

### 2 批量加载过程中如果出现错误，你的批量加载工具会怎么处理？不同错误处理方式是否有区别？请简要说明你的处理方式适用于什么场景，什么场景下不适用。

详见 `SM_Manager::Load()`，位于 `sm_manager.cc:513`：

- **情况 a：输入文件为空**  
  不认为是错误，不进行任何加载。

- **情况 b：列数错误**（第 515 行）  
  遇到错误行时立即终止加载。该策略适用于数据必须严格正确的场景。不适用于脏数据较多时想尽量加载正确数据的情况。

- **情况 c：属性类型不匹配**（第 513 行）  
  无法判断 ASCII 数据是否正确，因此依赖用户判断。例如，将浮点数截断为整数读取，或按需求将整数读取为浮点数；若将字符串误认为浮点或整数，将会进行二进制解释。  
  此行为继承自 `std::stringstream`。当需要严格类型匹配时，该策略不适用。若存在严格规则，需提供输入模式信息。

- **情况 d：插入记录或索引项失败**  
  遇到错误行时立即失败。不适用于要求全-or-无语义的场景，因为可能会出现部分加载状态。

---

### 3 当执行 `create index` 命令时，你是如何生成传入 `IX_Manager::CreateIndex()` 的文件名和索引号的？该方法是否有限制？

我使用列的偏移量作为索引号，使用 `-1` 表示没有索引。

参见 `SM_Manager::CreateIndex`，位于 `sm_manager.cc:336`：

#### 优点：

- 保证索引号是非负且唯一的，可区分各列
- 始终可用
- 属性长度和属性数量是固定的，因此不会无意中超出整型范围
- 无需扫描已有索引即可生成索引号

#### 缺点：

- 每个属性只能有一个索引（但 GuoShuBase 本身就有此限制）
- 如果支持 `alter table` 删除属性，将需要调整磁盘上其他列的所有偏移和索引号




## QL组件说明

### 参考资料

* 《Database Systems: The Complete Book》作者：Hector Garica-Molina, Jeffrey Ullman, Jennifer Widom
* 《Database Management Systems》作者：Raghu Ramakrishnan, Johannes Gehrke
* Minibase 文档：cs.wisc.edu

---

### 总体设计

本查询语言实现遵循迭代器模型。所有物理操作符均实现为迭代器，这些迭代器可彼此嵌套组合，从而实现灵活的查询计划构造。

对用户提交的查询进行了完整的语义检查，参见 `sm_manager.cc` 中的 `SM_Manager::SemCheck()` 方法。系统还会将 `select *` 语句和所有条件进行基础重写。

逻辑查询计划到物理查询计划的转换主要基于启发式方法。仅考虑**左深连接树**。选择计划时使用的主要统计信息包括：关系中的页数和记录数。连接顺序会优先选择较小的关系作为外部连接表。

只要条件允许，系统会优先使用索引。过滤器尽可能下推。无法继续下推的过滤器则在操作符输出端应用。索引扫描支持不同顺序（升序/降序），以便在 `<, >, =` 等操作中优化提前退出。

* 当右侧迭代器为索引扫描时，将使用 **嵌套循环索引连接（NestedLoopIndexJoin, NLIJ）**
* 当左侧迭代器为文件扫描时，将使用 **块嵌套循环连接（NestedBlockJoin）**
* 还实现了基本的 **嵌套循环连接（NestedLoopJoin）** 用于非叶子连接和交叉连接（笛卡尔积）

Insert 子句使用独立的逐记录实现，而非重用批量加载器，以减少对系统目录等元操作的频繁访问。

Update 子句单独实现，未复用 Delete/Insert 的逻辑，以实现**单次遍历（single-pass）**。为避免“**万圣节问题（Halloween Problem）**”，当更新属性即为索引属性时，不会选择索引扫描。

---

### 已实现的物理操作符

* FileScan（文件扫描）
* IndexScan（索引扫描）
* NestedLoopJoin（嵌套循环连接）
* Projection（投影）
* NestedLoopIndexJoin（继承自 NLJ）
* NestedBlockJoin（继承自 NLJ）

Filter 并非独立实现，而是集成在上述各操作符中。Projection 和 Join 可嵌套其他迭代器，实现灵活组合。

参见 `iterator.h`，该文件定义了所有操作符继承的 `Iterator` 接口及 `Tuple` 元组类。

---

### 测试情况

所有类均通过自动化单元测试进行测试。
各迭代器对应的测试文件见 `*_gtest.cc`。
另有完整 RQL 脚本测试 `ql_test.[1-7]`。

---

### 已知问题 / Bug

无特别记录的问题。

---

### 设计问答

#### 1. 查询优化器问题

**查询 Q：**

```sql
Select R.A, T.D
From  R, S, T
Where R.A = S.B and T.D = S.C
```

R.A 和 S.C 有索引。

示例数据表及索引：

```sql
create table stars(starid i, stname c20, plays c12, soapid i);
create table soaps(soapid i, sname c28, network c4, rating f);
create table networks(nid i, name c4, viewers i);

create index stars(soapid);
create index soaps(network);
```

查询：

```sql
select stars.soapid, networks.name
from stars, soaps, networks
where 
	stars.soapid = soaps.soapid
and	networks.name = soaps.network;
```

##### (a) 查询优化器生成的物理计划（Pretty-Print 格式）

```
Project
   nProjections = 2
      stars.soapid, networks.name
-----NestedLoopIndexJoin
   nJoinConds = 1
   joinConds[0]: stars.soapid = soaps.soapid
----------NestedLoopIndexJoin
   nJoinConds = 1
   joinConds[0]: networks.name = soaps.network
---------------FileScan
   relName = networks
---------------IndexScan
   relName = soaps
   attrName = network
----------IndexScan
   relName = stars
   attrName = soapid
```

所使用的统计信息仅包括：关系的记录数和页数。

##### (b) 优化器在语义检查后生成计划的步骤：

1. 按记录数对所有关系排序，记录数少的在前

   ```cpp
   ql_manager.cc:238
   ```

   排序结果示例：`(T, S, R)`

2. 始终采用左深连接树的查询形状

   ```cpp
   ql_manager.cc:250
   ```

3. 对于最左边（外层）关系选择 FileScan（若无合适索引）

   ```cpp
   QL_Manager::GetLeafIterator()
   ql_manager.cc:817
   ```

4. 对每个下一关系选择 IndexScan（如存在索引）

   ```cpp
   ql_manager.cc:280
   ```

5. 若右操作数为 IndexScan 且为连接条件，则使用 NestedLoopIndexJoin

   ```cpp
   ql_manager.cc:280
   ```

最终生成的计划结构为：

```
((FileScan T) NLIJ IndexScan S) NLIJ IndexScan R
```

##### (c) 是否使用了两个索引？是否存在同时使用两个索引的计划？

是的，计划中使用了两个索引。这是基于假设：连接选择率较小，使用索引查找是最高效的方式。优化器不会区分等值连接和交叉连接的策略，只是在非等值连接时禁用了某些计划（如 NLIJ）。

---

#### 2. 在执行 update/delete 时，如果有索引，是否使用？

会使用。调用逻辑与查询中一致：

* 使用 `QL_Manager::GetLeafIterator()` 判断是否能用索引

  ```cpp
  ql_manager.cc:817
  ```

* Delete 操作检查位置：

  ```cpp
  ql_manager.cc:541
  ```

* Update 操作检查位置：

  ```cpp
  ql_manager.cc:696
  ```

Update 特别处理了万圣节问题。

---

#### 3. 中间元组如何构造和传递？如何管理元组的 schema？

定义了 `Tuple` 类型用于中间元组传递，详见：

```cpp
iterator.h:18
```

所有操作符中的 `GetNext()` 方法都会使用 `Tuple`。
每个 `Tuple` 包含指向 `DataAttrInfo`（属性信息）的指针，用于描述其 Schema。
这些 Schema 信息由迭代器本身持有。

通常构造方式如下：

```cpp
Iterator it;
...
Tuple t = it.GetTuple();
```

例如可见：

```cpp
nested_block_join.h:59
NestedBlockJoin::GetNext() 中的 Tuple 使用
```

