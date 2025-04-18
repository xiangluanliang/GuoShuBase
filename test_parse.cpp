//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/4/18 1:03
// @Project: GuoShuBase
//
//
#include <iostream>
#include <memory>
#include "DBMSParser.h"
#include "Database.h"
#include "test.cpp"
void executeCommand(const DBMSParser::ParsedCommand& cmd, std::shared_ptr<Database>& currentDb) {
    switch (cmd.type) {
        case DBMSParser::CommandType::CREATE_DATABASE:
            if (!cmd.databaseName.empty()) {
                currentDb = std::make_shared<Database>(cmd.databaseName);
                TestPF();
                std::cout << "数据库 " << cmd.databaseName << " 创建成功" << std::endl;
            }
            break;

        case DBMSParser::CommandType::DROP_DATABASE:
            if (currentDb && currentDb->getName() == cmd.databaseName) {
                currentDb.reset();
                std::cout << "数据库 " << cmd.databaseName << " 删除成功" << std::endl;
            }
            break;

        case DBMSParser::CommandType::CREATE_TABLE:
            if (currentDb) {
                currentDb->createTable(cmd.tableName, cmd.columns);
            }
            break;

        case DBMSParser::CommandType::DROP_TABLE:
            if (currentDb) {
                currentDb->dropTable(cmd.tableName);
            }
            break;

        case DBMSParser::CommandType::ALTER_TABLE:
            if (currentDb) {
                currentDb->alterTable(cmd.tableName, cmd.alterationType, cmd.alterationColumn);
            }
            break;

        default:
            std::cout << "未知命令" << std::endl;
            break;
    }
}

int main() {
    std::shared_ptr<Database> currentDb;
    std::string sql;

    std::cout << "简单DBMS系统启动" << std::endl;
    std::cout << "支持的命令:" << std::endl;
    std::cout << "1. CREATE DATABASE dbname" << std::endl;
    std::cout << "2. DROP DATABASE dbname" << std::endl;
    std::cout << "3. CREATE TABLE tablename (column1 type1, column2 type2, ...)" << std::endl;
    std::cout << "4. DROP TABLE tablename" << std::endl;
    std::cout << "5. ALTER TABLE tablename ADD column type" << std::endl;
    std::cout << "6. ALTER TABLE tablename DROP column" << std::endl;
    std::cout << "7. ALTER TABLE tablename MODIFY column type" << std::endl;
    std::cout << "输入 'exit' 退出" << std::endl;

//    TestPF();
    while (true) {
        std::cout << "\nGuoShuBase> ";
        std::getline(std::cin, sql);

        if (sql == "exit") {
            exit(0);
        }

        if (!sql.empty()) {
            auto command = DBMSParser::parse(sql);
            executeCommand(command, currentDb);
        }
    }

    return 0;
}