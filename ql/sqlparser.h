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
enum class SQLType {
    SELECT, INSERT, DELETE, UPDATE,
    CREATE_TABLE, DROP_TABLE,
    CREATE_INDEX, DROP_INDEX,
    HELP, PRINT, LOAD,ALTER_TABLE,
    SHOW_INDEX
};


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
    std::vector<Value> values;

    // DELETE
    char* deleteTableName = nullptr;

    // UPDATE
    char* updateTableName = nullptr;
    RelAttr updateAttr;
    bool updateIsValue = true;
    Value updateValue;
    RelAttr updateRhsAttr;

    // CREATE TABLE
    char* createTableName = nullptr;
    std::vector<AttrInfo> attrList;

    // DROP TABLE
    char* dropTableName = nullptr;

    // CREATE INDEX / DROP INDEX
    char* indexTableName = nullptr;
    char* indexAttrName = nullptr;
//    std::vector<char*> indexAttrs;

    // ALTER TABLE
    char* alterTableName = nullptr;


    //  // HELP / PRINT
//    char* helpTableName = nullptr;
//    char* printTableName = nullptr;
//
//  // LOAD
//    char* loadTableName = nullptr;
//    char* loadFileName = nullptr;

    ~ParsedQuery() = default;
};

RC ParseSQL(const std::vector<Token>& tokens, ParsedQuery& out);

#endif // SQLPARSER_H
