#ifndef STATISTICS_H
#define STATISTICS_H

#ifndef Boolean
typedef char Boolean;  // 布尔类型
#endif

#ifndef TRUE
#define TRUE 1        // 真值
#endif

#ifndef FALSE
#define FALSE 0       // 假值
#endif

#ifndef NULL
#define NULL 0        // 空指针
#endif

#ifndef RC
typedef int RC;       // 返回码类型
#endif

#ifndef STAT_BASE
const int STAT_BASE = 9000;  // 统计模块错误码基值
#endif

// 必须在通用定义后包含链表模板类
#include "linkedlist.h"  // 链表模板类

//----------------------------------------------
// Statistic类 - 单个统计项的追踪
//----------------------------------------------
class Statistic {
public:
    Statistic();  // 默认构造函数
    ~Statistic(); // 析构函数
    Statistic(const char *psName); // 带名称的构造函数

    // 拷贝构造函数
    Statistic(const Statistic &stat);

    // 赋值运算符重载
    Statistic& operator=(const Statistic &stat);

    // 比较运算符重载(基于统计项名称)
    Boolean operator==(const char *psName_) const;

    char *psKey;   // 统计项的名称/键值
    int iValue;    // 统计值(当前仅支持整型值，初始为0)
};

//----------------------------------------------
// 统计操作枚举类型
// 表示可以对统计项执行的各种操作
//----------------------------------------------
enum Stat_Operation {
    STAT_ADDONE,    // 值加1
    STAT_ADDVALUE,  // 增加指定值
    STAT_SETVALUE,  // 设置为指定值
    STAT_MULTVALUE, // 乘以指定值
    STAT_DIVVALUE,  // 除以指定值
    STAT_SUBVALUE   // 减去指定值
};

//----------------------------------------------
// StatisticsMgr类 - 管理一组统计项
//----------------------------------------------
class StatisticsMgr {
public:
    StatisticsMgr() {};  // 构造函数
    ~StatisticsMgr() {}; // 析构函数

    // 添加新统计项或更新已有统计项
    // piValue可以为NULL(除需要该参数的操作外)
    // 添加时默认值为0，然后执行指定操作
    RC Register(const char *psKey, const Stat_Operation op,
                const int *const piValue = NULL);

    // 获取指定统计项的值
    // 调用者负责释放返回的内存
    int *Get(const char *psKey);

    // 打印单个统计项
    RC Print(const char *psKey);

    // 打印所有统计项
    void Print();

    // 重置单个统计项
    RC Reset(const char *psKey);

    // 重置所有统计项
    void Reset();

private:
    LinkList<Statistic> llStats;  // 统计项链表(使用模板类)
};

//----------------------------------------------
// 返回码定义
//----------------------------------------------
const int STAT_INVALID_ARGS = STAT_BASE+1;  // 方法调用参数无效
const int STAT_UNKNOWN_KEY  = STAT_BASE+2;  // 未知的统计键值

//----------------------------------------------
// Redbase的PF组件专用统计键值
// 当使用统计功能时，这些常量将作为统计管理器的键值
//----------------------------------------------
extern const char *PF_GETPAGE;      // 获取页面次数
extern const char *PF_PAGEFOUND;    // 页面命中次数
extern const char *PF_PAGENOTFOUND; // 页面未命中次数
extern const char *PF_READPAGE;     // 页面读取IO次数
extern const char *PF_WRITEPAGE;    // 页面写入IO次数
extern const char *PF_FLUSHPAGES;   // 页面刷新次数

#endif // STATISTICS_H
