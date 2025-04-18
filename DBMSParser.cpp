//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 1:01
// @Project: GuoShuBase
//
//

#include "DBMSParser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

DBMSParser::ParsedCommand DBMSParser::parse(const std::string& sql) {
    ParsedCommand cmd;
    std::vector<std::string> tokens = tokenize(sql);

    if (tokens.size() < 2) {
        cmd.type = CommandType::UNKNOWN;
        return cmd;
    }

    // 转换为大写以进行比较
    std::string firstToken = tokens[0];
    std::transform(firstToken.begin(), firstToken.end(), firstToken.begin(), ::toupper);
    std::string secondToken = tokens[1];
    std::transform(secondToken.begin(), secondToken.end(), secondToken.begin(), ::toupper);

    if (firstToken == "CREATE") {
        if (secondToken == "DATABASE") {
            cmd.type = CommandType::CREATE_DATABASE;
            if (tokens.size() >= 3) {
                cmd.databaseName = tokens[2];
            }
        }
        else if (secondToken == "TABLE") {
            cmd.type = CommandType::CREATE_TABLE;
            if (tokens.size() >= 3) {
                cmd.tableName = tokens[2];
                // 解析列定义
                size_t i = 4; // 跳过 ( 符号
                while (i < tokens.size() && tokens[i] != ")") {
                    if (tokens[i] != ",") {
                        Column col;
                        col.name = tokens[i];
                        if (i + 1 < tokens.size()) {
                            col.type = getDataType(tokens[i + 1]);
                            cmd.columns.push_back(col);
                        }
                        i += 2;
                    }
                    else {
                        i++;
                    }
                }
            }
        }
    }
    else if (firstToken == "DROP") {
        if (secondToken == "DATABASE") {
            cmd.type = CommandType::DROP_DATABASE;
            if (tokens.size() >= 3) {
                cmd.databaseName = tokens[2];
            }
        }
        else if (secondToken == "TABLE") {
            cmd.type = CommandType::DROP_TABLE;
            if (tokens.size() >= 3) {
                cmd.tableName = tokens[2];
            }
        }
    }
    else if (firstToken == "ALTER" && secondToken == "TABLE") {
        cmd.type = CommandType::ALTER_TABLE;
        if (tokens.size() >= 5) {
            cmd.tableName = tokens[2];
            std::string alterCmd = tokens[3];
            std::transform(alterCmd.begin(), alterCmd.end(), alterCmd.begin(), ::toupper);
            cmd.alterationType = alterCmd;

            Column col;
            col.name = tokens[4];
            if (tokens.size() >= 6) {
                col.type = getDataType(tokens[5]);
            }
            cmd.alterationColumn = col;
        }
    }

    return cmd;
}

std::vector<std::string> DBMSParser::tokenize(const std::string& sql) {
    std::vector<std::string> tokens;
    std::istringstream iss(sql);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

DataType DBMSParser::getDataType(const std::string& type) {
    std::string upperType = type;
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);

    if (upperType == "INT") return DataType::INT;
    if (upperType == "VARCHAR") return DataType::VARCHAR;
    if (upperType == "FLOAT") return DataType::FLOAT;

    return DataType::VARCHAR; // 默认类型
}