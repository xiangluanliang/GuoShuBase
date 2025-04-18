//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 1:01
// @Project: GuoShuBase
//
//

#ifndef GUOSHUBASE_DBMSPARSER_H
#define GUOSHUBASE_DBMSPARSER_H


#include <string>
#include <vector>
#include "Types.h"

class DBMSParser {
public:
    enum class CommandType {
        CREATE_DATABASE,
        DROP_DATABASE,
        CREATE_TABLE,
        DROP_TABLE,
        ALTER_TABLE,
        UNKNOWN
    };

    struct ParsedCommand {
        CommandType type;
        std::string databaseName;
        std::string tableName;
        std::vector<Column> columns;
        std::string alterationType;  // ADD, DROP, MODIFY
        Column alterationColumn;
    };

    static ParsedCommand parse(const std::string& sql);

private:
    static std::vector<std::string> tokenize(const std::string& sql);
    static DataType getDataType(const std::string& type);
};


#endif //GUOSHUBASE_DBMSPARSER_H
