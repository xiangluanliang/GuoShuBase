//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 0:59
// @Project: GuoShuBase
//
//

#ifndef GUOSHUBASE_TABLE_H
#define GUOSHUBASE_TABLE_H

#include "Types.h"
#include <string>
#include <vector>

class Table {
public:
    Table(const std::string& name, const std::vector<Column>& columns);
    bool addColumn(const Column& column);
    bool dropColumn(const std::string& columnName);
    bool modifyColumn(const Column& column);
    const std::vector<Column>& getColumns() const;
    std::string getName() const;

private:
    std::string name;
    std::vector<Column> columns;
};


#endif //GUOSHUBASE_TABLE_H
