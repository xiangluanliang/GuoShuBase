//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 0:58
// @Project: GuoShuBase
//
//

#include "DataBase.h"
#include <iostream>
#include <filesystem>

Database::Database(const std::string& name) : name(name) {}

bool Database::createTable(const std::string& tableName, const std::vector<Column>& columns) {
    if (tables.find(tableName) != tables.end()) {
        std::cout << "表 " << tableName << " 已存在" << std::endl;
        return false;
    }

    tables[tableName] = std::make_shared<Table>(tableName, columns);
    std::cout << "表 " << tableName << " 创建成功" << std::endl;
    return true;
}

bool Database::dropTable(const std::string& tableName) {
    if (tables.find(tableName) == tables.end()) {
        std::cout << "表 " << tableName << " 不存在" << std::endl;
        return false;
    }

    tables.erase(tableName);
    std::cout << "表 " << tableName << " 删除成功" << std::endl;
    return true;
}

std::shared_ptr<Table> Database::getTable(const std::string& tableName) {
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return it->second;
    }
    return nullptr;
}

bool Database::alterTable(const std::string& tableName, const std::string& operation,
                          const Column& column) {
    auto table = getTable(tableName);
    if (!table) {
        std::cout << "表 " << tableName << " 不存在" << std::endl;
        return false;
    }

    if (operation == "ADD") {
        return table->addColumn(column);
    }
    else if (operation == "DROP") {
        return table->dropColumn(column.name);
    }
    else if (operation == "MODIFY") {
        return table->modifyColumn(column);
    }

    return false;
}