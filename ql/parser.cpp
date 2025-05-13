//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/10 22:40
// @Project: GuoShuBase
//
//


#include "parser.h"
#include "sqlparser.h"
#include "tokenizer.h"
#include "rm.h"
#include "ix.h"
#include "ql.h"
#include <cstring>

//
// PrintError
//
// Desc: Print an error message by calling the proper component-specific
//       print-error function
//
void PrintError(RC rc) {
    if (abs(rc) <= END_PF_WARN)
        PF_PrintError(rc);
    else if (abs(rc) <= END_RM_WARN)
        RM_PrintError(rc);
    else if (abs(rc) <= END_IX_WARN)
        IX_PrintError(rc);
    else if (abs(rc) <= END_SM_WARN)
        SM_PrintError(rc);
    else if (abs(rc) <= END_QL_WARN)
        QL_PrintError(rc);
    else
        cerr << "Error code out of range: " << rc << "\n";
}

// 如果某个 attrName 出现在多张表中则抛异常
RC ResolveAttrRelName(SM_Manager &smm,
                      const std::vector<char*> &relations,
                      const char *attrName,
                      char *&relNameOut) {
    if(strcmp(attrName,"*") == 0) return 0;
    RC rc;
    relNameOut = nullptr;

    DataAttrInfo attr;
    RID rid;

    for (char *rel : relations) {
        rc = smm.GetAttrFromCat(rel, attrName, attr, rid);
//        cerr << rel << ":" << attrName << endl;
        if (rc == 0) {
            if (relNameOut != nullptr) {
                // 已经找到过一次返回冲突错误
//                PrintError(QL_AMBIGUOUSATTR);
                return QL_AMBIGUOUSATTR;
            }
            relNameOut = rel; // 暂存
        } else if (rc != SM_BADATTR) {
            // 其他错误（不是找不到字段），直接返回
            return rc;
        }
        // rc == SM_BADATTR 继续找下一张表
    }

    if (!relNameOut) {
//        PrintError(QL_ATTRNOTFOUND);
        return QL_ATTRNOTFOUND;
    }

    return 0;
}



RC GBparseSQL(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm, const char *sql) {
    RC rc;
    if(strcmp(sql,"exit") == 0) return EXITSYS;
    std::vector<Token> tokens = Tokenize(sql);
//    for (auto t:tokens) {
//        cout << TokenTypeToString(t.type) << ": " << t.text << endl;
//    }
    ParsedQuery query;
    if ((rc = ParseSQL(tokens, query))) {
        return rc;
    }

    // 用于释放临时分配的堆内存
    std::vector<AggRelAttr> selAttrBuffer;
    std::vector<const char *> relNameBuffer;

    switch (query.type) {
        case SQLType::SELECT: {
            char *resolvedRelName;
            for (const auto &srcAttr : query.selAttrs) {
                AggRelAttr attr;
                attr.func = srcAttr.func;
                if (srcAttr.relName) {
                    attr.relName = strdup(srcAttr.relName);
                } else {
                    resolvedRelName = nullptr;
                    if ((rc = ResolveAttrRelName(smm, query.relations, srcAttr.attrName, resolvedRelName)))
                        return rc;
                    attr.relName = strdup(resolvedRelName);
                }
                attr.attrName = srcAttr.attrName ? strdup(srcAttr.attrName) : strdup("");
                selAttrBuffer.push_back(attr);
            }

            for (char *r : query.relations)
                relNameBuffer.push_back(r);

            // 修复 ORDER BY 字段
            RelAttr orderAttr = {strdup(""), strdup("")};
            if (query.hasOrderBy && !query.orderAttrs.empty()) {
                orderAttr = query.orderAttrs[0];
                if (!orderAttr.relName) {
                    resolvedRelName = nullptr;
                    if ((rc = ResolveAttrRelName(smm, query.relations, orderAttr.attrName, resolvedRelName)))
                        return rc;
                    orderAttr.relName = strdup(resolvedRelName);
                }
            }

            // 修复 GROUP BY 字段
            RelAttr groupAttr = {strdup(""), strdup("")};
            if (query.hasGroupBy) {
                groupAttr = query.groupAttr;
                if (!groupAttr.relName) {
                    resolvedRelName = nullptr;
                    if ((rc = ResolveAttrRelName(smm, query.relations, groupAttr.attrName, resolvedRelName)))
                        return rc;
                    groupAttr.relName = strdup(resolvedRelName);
                }
            }

            // 修复 WHERE 子句字段
            for (Condition& cond : query.conditions) {
                if (!cond.lhsAttr.relName) {
                    resolvedRelName = nullptr;
                    if ((rc = ResolveAttrRelName(smm, query.relations, cond.lhsAttr.attrName, resolvedRelName)))
                        return rc;
                    cond.lhsAttr.relName = strdup(resolvedRelName);
                }
                if (cond.bRhsIsAttr && !cond.rhsAttr.relName) {
                    resolvedRelName = nullptr;
                    if ((rc = ResolveAttrRelName(smm, query.relations, cond.rhsAttr.attrName, resolvedRelName)))
                        return rc;
                    cond.rhsAttr.relName = strdup(resolvedRelName);
                }
            }

            return qlm.Select(
                    selAttrBuffer.size(),
                    selAttrBuffer.data(),
                    relNameBuffer.size(),
                    relNameBuffer.data(),
                    query.conditions.size(),
                    query.conditions.data(),
                    query.hasOrderBy ? 1 : 0,
                    orderAttr,
                    query.hasGroupBy,
                    groupAttr
            );
        }

        case SQLType::INSERT: {
            const char *relName = query.insertTableName ? query.insertTableName : "dummy";
            int nValues = query.values.size();
            const Value *values = nValues ? query.values.data() : nullptr;
            return qlm.Insert(relName, nValues, values);
        }

        case SQLType::DELETE: {
            const char *relName = query.deleteTableName ? query.deleteTableName : "dummy";
            int nConditions = query.conditions.size();
            const Condition *conditions = nConditions ? query.conditions.data() : nullptr;
            return qlm.Delete(relName, nConditions, conditions);
        }

        case SQLType::UPDATE: {
            const char *relName = query.updateTableName ? query.updateTableName : "dummy";
            int bIsValue = query.updateIsValue ? 1 : 0;

            static int dummyInt = 0;
            const Value &rhsValue = query.updateIsValue
                                    ? query.updateValue
                                    : Value{INT, &dummyInt};

            return qlm.Update(
                    relName,
                    query.updateAttr,
                    bIsValue,
                    bIsValue ? RelAttr{strdup(""), strdup("")} : query.updateRhsAttr,
                    rhsValue,
                    query.conditions.size(),
                    query.conditions.data()
            );
        }

        case SQLType::CREATE_TABLE: {
            return smm.CreateTable(
                    query.createTableName,
                    query.attrList.size(),
                    query.attrList.data()
            );
        }

        case SQLType::DROP_TABLE:
            return smm.DropTable(query.dropTableName);

        default:
            return QL_INVALIDQUERY;
    }

    return 0;
}



RC GBparse(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm) {
    RC rc = 0;
    while (true) {
        cout << PROMPT;
        string input;
        if(!getline(cin, input)) break;
        if((rc = GBparseSQL(pfm, smm, qlm, input.c_str()))){
            if(rc == 42) return rc;
            PrintError(rc);
            continue;
        };
    }
    return rc;
}

//
// Functions for printing the various structures to an output stream
//
ostream &operator<<(ostream &s, const AttrInfo &ai) {
    return
            s << " attrName=" << ai.attrName
              << " attrType=" <<
              (ai.attrType == INT ? "INT" :
               ai.attrType == FLOAT ? "FLOAT" : "STRING")
              << " attrLength=" << ai.attrLength;
}

ostream &operator<<(ostream &s, const RelAttr &qa) {
    return
            s << (qa.relName ? qa.relName : "NULL")
              << "." << qa.attrName;
}

ostream &operator<<(ostream &s, const AggRelAttr &qa) {
    if (qa.func == NO_F)
        return
                s << (qa.relName ? qa.relName : "NULL")
                  << "." << qa.attrName;
    else
        return
                s << qa.func << "(" << (qa.relName ? qa.relName : "NULL")
                  << "." << qa.attrName << ")";
}

ostream &operator<<(ostream &s, const Condition &c) {
    s << "\n      lhsAttr:" << c.lhsAttr << "\n"
      << "      op=" << c.op << "\n";
    if (c.bRhsIsAttr)
        s << "      bRhsIsAttr=TRUE \n      rhsAttr:" << c.rhsAttr;
    else
        s << "      bRshIsAttr=FALSE\n      rhsValue:" << c.rhsValue;
    return s;
}

ostream &operator<<(ostream &s, const Value &v) {
    s << "AttrType: " << v.type;
    switch (v.type) {
        case INT:
            s << " *(int *)data=" << *(int *) v.data;
            break;
        case FLOAT:
            s << " *(float *)data=" << *(float *) v.data;
            break;
        case STRING:
            s << " (char *)data=" << (char *) v.data;
            break;
    }
    return s;
}

ostream &operator<<(ostream &s, const AggFun &func) {
    switch (func) {
        case MIN_F:
            s << " MIN";
            break;
        case MAX_F:
            s << " MAX";
            break;
        case AVG_F:
            s << " AVG";
            break;
        case SUM_F:
            s << " SUM";
            break;
        case COUNT_F:
            s << " COUNT";
            break;
        default:
            // case NO_F:
            s << " ";
            break;
    }
    return s;
}

ostream &operator<<(ostream &s, const CompOp &op) {
    switch (op) {
        case EQ_OP:
            s << " =";
            break;
        case NE_OP:
            s << " <>";
            break;
        case LT_OP:
            s << " <";
            break;
        case LE_OP:
            s << " <=";
            break;
        case GT_OP:
            s << " >";
            break;
        case GE_OP:
            s << " >=";
            break;
        case NO_OP:
            s << " NO_OP";
            break;
    }
    return s;
}

ostream &operator<<(ostream &s, const AttrType &at) {
    switch (at) {
        case INT:
            s << "INT";
            break;
        case FLOAT:
            s << "FLOAT";
            break;
        case STRING:
            s << "STRING";
            break;
    }
    return s;
}
