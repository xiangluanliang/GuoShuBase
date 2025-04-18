//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 0:58
// @Project: GuoShuBase
//
//

#ifndef GUOSHUBASE_DATABASE_H
#define GUOSHUBASE_DATABASE_H


#include <string>
#include <memory>
#include "Table.h"
#include <unordered_map>

class Database {
public:
    Database(const std::string& name);
    bool createTable(const std::string& tableName, const std::vector<Column>& columns);
    bool dropTable(const std::string& tableName);
    std::shared_ptr<Table> getTable(const std::string& tableName);
    bool alterTable(const std::string& tableName, const std::string& operation,
                    const Column& column);
    std::string getName() const { return name; }

private:
    std::string name;
    std::unordered_map<std::string, std::shared_ptr<Table>> tables;
};


#endif //GUOSHUBASE_DATABASE_H
