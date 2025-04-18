//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 0:59
// @Project: GuoShuBase
//
//

#include "Table.h"
#include "Table.h"
#include <algorithm>
#include <iostream>

Table::Table(const std::string& name, const std::vector<Column>& columns)
        : name(name), columns(columns) {
}

bool Table::addColumn(const Column& column) {
    auto it = std::find_if(columns.begin(), columns.end(),
                           [&](const Column& col) { return col.name == column.name; });

    if (it != columns.end()) {
        std::cout << "列 " << column.name << " 已存在" << std::endl;
        return false;
    }

    columns.push_back(column);
    std::cout << "列 " << column.name << " 添加成功" << std::endl;
    return true;
}

bool Table::dropColumn(const std::string& columnName) {
    auto it = std::find_if(columns.begin(), columns.end(),
                           [&](const Column& col) { return col.name == columnName; });

    if (it == columns.end()) {
        std::cout << "列 " << columnName << " 不存在" << std::endl;
        return false;
    }

    columns.erase(it);
    std::cout << "列 " << columnName << " 删除成功" << std::endl;
    return true;
}

bool Table::modifyColumn(const Column& column) {
    auto it = std::find_if(columns.begin(), columns.end(),
                           [&](const Column& col) { return col.name == column.name; });

    if (it == columns.end()) {
        std::cout << "列 " << column.name << " 不存在" << std::endl;
        return false;
    }

    *it = column;
    std::cout << "列 " << column.name << " 修改成功" << std::endl;
    return true;
}

const std::vector<Column>& Table::getColumns() const {
    return columns;
}

std::string Table::getName() const {
    return name;
}