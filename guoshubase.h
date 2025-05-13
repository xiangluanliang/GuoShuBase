//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/3/13 18:07
// @Project: GuoShuBase
//
// 本文件定义了一些最基本的数据规则和返回值规则，包括常量、枚举和错误代码等。

#ifndef GUOSHUBASE_H
#define GUOSHUBASE_H

#define MAXNAME       24                // 关系或属性名（即表名和列名）的最大长度
#define MAXSTRINGLEN  255               // 字符串类型属性的最大长度
#define MAXATTRS      40                // 关系中最大属性数（即表中最大列数）

// Return codes
typedef int RC;

#define OK_RC         0    // OK_RC 返回值永远保证为0
#define EXITSYS       42   // 退出系统
//错误码 Error Code Ranges
#define START_PF_ERR  (-1)
#define END_PF_ERR    (-100)
#define START_RM_ERR  (-101)
#define END_RM_ERR    (-200)
#define START_IX_ERR  (-201)
#define END_IX_ERR    (-300)
#define START_SM_ERR  (-301)
#define END_SM_ERR    (-400)
#define START_QL_ERR  (-401)
#define END_QL_ERR    (-500)

//警告码 Warning Code Ranges
#define START_PF_WARN  1
#define END_PF_WARN    100
#define START_RM_WARN  101
#define END_RM_WARN    200
#define START_IX_WARN  201
#define END_IX_WARN    300
#define START_SM_WARN  301
#define END_SM_WARN    400
#define START_QL_WARN  401
#define END_QL_WARN    500

// ALL_PAGES is defined and used by the ForcePages method defined in RM
// and PF layers
const int ALL_PAGES = -1;

// 属性类型
enum AttrType {
    INT,
    FLOAT,
    STRING
};

// 比较运算符(binary atomic operators)
enum CompOp {
    NO_OP,         // No comparison
    EQ_OP, NE_OP,  // Equal, Not Equal
    LT_OP, GT_OP,  // Less than, Greater than
    LE_OP, GE_OP   // Less than or Equal, Greater than or Equal
};

enum AggFun {
    NO_F,
    MIN_F, MAX_F, COUNT_F,
    SUM_F, AVG_F           // numeric args only
};
// 不太懂，可能用于缓冲区管理的，Mark一下，用到了再说，没用到就删了
// Pin Strategy Hint
//
enum ClientHint {
    NO_HINT                                     // default value
};

// 最意义不明的一集，看了一圈就这两行不知道干什么的，不明觉厉不敢删，Mark吧
#define yywrap() 1
void yyerror(const char *);

// 兼容性/SQL语义处理，先留着，应该会用上，Mark
// TRUE, FALSE and BOOLEAN
//
#ifndef BOOLEAN
typedef char Boolean;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#endif //GUOSHUBASE_GUOSHUBASE_H
