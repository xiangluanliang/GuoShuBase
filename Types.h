//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 1:00
// @Project: GuoShuBase
//
//

#ifndef GUOSHUBASE_TYPES_H
#define GUOSHUBASE_TYPES_H
#include <string>
#include <vector>

enum class DataType {
    INT,
    VARCHAR,
    FLOAT
};

struct Column {
    std::string name;
    DataType type;
    int length;  // 用于VARCHAR类型的长度限制
};

struct TableInfo {
    std::string name;
    std::vector<Column> columns;
};
#endif //GUOSHUBASE_TYPES_H
