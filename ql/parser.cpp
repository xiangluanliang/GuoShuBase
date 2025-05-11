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


RC GBparseSQL(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm, const char *sql) {

    std::vector<Token> tokens;
    try {
        tokens = Tokenize(sql);
    } catch (exception e) {
        std::cerr << e.what() << std::endl;
        return QL_INVALIDQUERY;
    }

    ParsedQuery query;
    if (!ParseSQL(tokens, query)) {
        return QL_INVALIDQUERY;
    }
    for (auto t : query.selAttrs) {
        cout << t << endl;
    }for (auto t : query.relations) {
        cout << t << endl;
    }





    switch (query.type) {
        case SQLType::SELECT: {
            // 转换 SELECT 查询参数
            int nSelAttrs = query.selAttrs.size();
            AggRelAttr *selAttrs = query.selAttrs.data();

            int nRelations = query.relations.size();
            const char **relations = const_cast<const char **>(query.relations.data());

            int nConditions = query.conditions.size();
            Condition *conditions = query.conditions.data();

            // 处理 ORDER BY
            int order = query.hasOrderBy ? 1 : 0;
            RelAttr orderAttr = query.hasOrderBy && !query.orderAttrs.empty() ?
                                query.orderAttrs[0] : RelAttr{nullptr, nullptr};

            // 处理 GROUP BY
            bool group = query.hasGroupBy;
            RelAttr groupAttr = query.groupAttr;

            return qlm.Select(nSelAttrs, selAttrs, nRelations, relations,
                              nConditions, conditions, order, orderAttr, group, groupAttr);
        }

        case SQLType::INSERT: {
            return qlm.Insert(query.insertTableName,
                              query.values.size(),
                              query.values.data());
        }

        case SQLType::DELETE: {
            return qlm.Delete(query.deleteTableName,
                              query.conditions.size(),
                              query.conditions.data());
        }

        case SQLType::UPDATE: {
            return qlm.Update(query.updateTableName,
                              query.updateAttr,
                              query.updateIsValue ? 1 : 0,
                              query.updateRhsAttr,
                              query.updateValue,
                              query.conditions.size(),
                              query.conditions.data());
        }

        default:
            return QL_INVALIDQUERY;
    }

}


RC GBparse(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm) {
    RC rc = 0;
    while (true) {
        cout << PROMPT;
        string input;
        if (!getline(cin, input)) break;

        std::cout<< input.c_str() <<std::endl;
        
        rc = GBparseSQL(pfm, smm, qlm, input.c_str());
        if (rc != QL_INVALIDQUERY) {
            if (rc) PrintError(rc);
            continue;
        }
    }
    return rc;
}

void FreeAttrInfo(AttrInfo &ai) {
    free(ai.attrName);
    ai.attrName = nullptr;
}

void FreeRelAttr(RelAttr &ra) {
    free(ra.relName);
    free(ra.attrName);
    ra.relName = ra.attrName = nullptr;
}

void FreeValue(Value &v) {
    if (v.data) {
        if (v.type == STRING) free(v.data);
        else if (v.type == INT) delete (int *) v.data;
        else if (v.type == FLOAT) delete (float *) v.data;
        v.data = nullptr;
    }
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
