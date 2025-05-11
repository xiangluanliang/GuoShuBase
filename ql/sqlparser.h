//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/10 21:52
// @Project: GuoShuBase
//
//
#ifndef SQLPARSER_H
#define SQLPARSER_H

#include "tokenizer.h"
#include "parser.h"
#include "../guoshubase.h"
#include <vector>

#define PROMPT "\nGUOSHUBASE >> "
enum class SQLType { SELECT, INSERT, DELETE, UPDATE };

struct ParsedQuery {
    SQLType type;

    // SELECT
    std::vector<AggRelAttr> selAttrs;
    std::vector<char*> relations;
    std::vector<Condition> conditions;
    bool isDistinct = false;

    bool hasGroupBy = false;
    RelAttr groupAttr;

    bool hasOrderBy = false;
    std::vector<RelAttr> orderAttrs;

    // INSERT
    char* insertTableName = nullptr;
    std::vector<char*> insertFields;
    std::vector<Value> values;

    // DELETE
    char* deleteTableName = nullptr;

    // UPDATE
    char* updateTableName = nullptr;
    RelAttr updateAttr;
    bool updateIsValue = true;
    Value updateValue;
    RelAttr updateRhsAttr;

    ~ParsedQuery() {
        // 释放所有动态分配的内存
        for (char* rel : relations) free(rel);
        free(insertTableName);
        for (char* field : insertFields) free(field);
        free(deleteTableName);
        free(updateTableName);
    }
};

bool ParseSQL(const std::vector<Token>& tokens, ParsedQuery& out);

#endif // SQLPARSER_H
