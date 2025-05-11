//
// ql.h
//   Query Language Component Interface
//


#ifndef QL_H
#define QL_H

#include <stdlib.h>
#include <string.h>
#include "../guoshubase.h"
#include "parser.h"
#include "ql_error.h"
#include "rm.h"
#include "ix.h"
#include "sm.h"
#include "iterator.h"

//
// DML部分
//

class QL_Manager {
public:
    QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);

    ~QL_Manager();                               // Destructor

    RC Select(int nSelAttrs,                   // 选择的字段数量
            //              const RelAttr selAttrs[],        // attrs in select clause
              const AggRelAttr selAttrs[],     // 选择的字段列表（支持聚合函数，如 COUNT(*)）
              int nRelations,                // 涉及的表数量（支持 join）
              const char *const relations[],  // 表名数组
              int nConditions,               // WHERE 条件数量
              const Condition conditions[],    // WHERE 条件数组
              int order,                       // 是否有 ORDER BY
              RelAttr orderAttr,               // 排序的字段
              bool group, //是否有 GROUP BY
              RelAttr groupAttr); //分组字段

    RC Insert(const char *relName,             // 目标表名
              int nValues,                   // 值的数量
              const Value values[]);           // 值的数组（与表字段顺序对应）

    RC Delete(const char *relName,             // 目标表名
              int nConditions,               // WHERE 条件数量
              const Condition conditions[]);   // WHERE 条件数组

    RC Update(const char *relName,             // 目标表名
              const RelAttr &updAttr,          // 要更新的字段
              const int bIsValue,              // 如果为 1，表示右侧是值；为 0 表示右侧是另一个字段
              const RelAttr &rhsRelAttr,       // 右侧字段（当 bIsValue 为 0 时）
              const Value &rhsValue,           // 右侧值（当 bIsValue 为 1 时）
              int nConditions,               // WHERE 条件数量
              const Condition conditions[]);   // WHERE 条件数组
public:
    RC IsValid() const;

    // 创建底层的记录迭代器（例如全表扫描或索引扫描）
    //
    // 用于构造执行计划树的底部节点。
    Iterator *GetLeafIterator(const char *relName,
                              int nConditions,
                              const Condition conditions[],
                              int nJoinConditions = 0,
                              const Condition jconditions[] = NULL,
                              int order = 0,
                              RelAttr *porderAttr = NULL);

    RC MakeRootIterator(Iterator *&newit,
                        int nSelAttrs, const AggRelAttr selAttrs[],
                        int nRelations, const char *const relations[],
                        int order, RelAttr orderAttr,
                        bool group, RelAttr groupAttr) const;

    RC PrintIterator(Iterator *it) const;

    void GetCondsForSingleRelation(int nConditions,
                                   Condition conditions[],
                                   char *relName,
                                   int &retCount, Condition *&retConds) const;

    // 从 WHERE 条件中提取只涉及某一个表的条件
    //
    // 用于提前下推条件，加快执行效率。
    void GetCondsForTwoRelations(int nConditions,
                                 Condition conditions[],
                                 int nRelsSoFar,
                                 char *relations[],
                                 char *relName2,
                                 int &retCount, Condition *&retConds) const;

private:
    RM_Manager &rmm;
    IX_Manager &ixm;
    SM_Manager &smm;
};

#endif // QL_H
