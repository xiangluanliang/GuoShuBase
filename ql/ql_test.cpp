//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/10 21:39
// @Project: GuoShuBase
//
//


#include <string>
#include <iostream>
#include "tokenizer.h"
#include "sqlparser.h"

int main() {
    std::vector<std::string> test_sqls = {
            "INSERT INTO student VALUES (1, 'Alice', 18);",
            "INSERT INTO student (id, name, age) VALUES (2, 'Bob', 19);",
            "SELECT sno, age FROM student WHERE age > 18;",
            "DELETE FROM student WHERE id = 2;",
            "UPDATE student SET age = 20 WHERE name = 'Alice';"
    };

    for (const auto &sql: test_sqls) {
        std::cout << "===== SQL =====\n" << sql << "\n";
        auto tokens = Tokenize(sql);
        ParsedQuery pq;
        if (ParseSQL(tokens, pq)) {
            std::cout << "Parse successful!\n";
            switch (pq.type) {
                case SQLType::SELECT:
                    std::cout << "Query type: SELECT, ";
                    std::cout << pq.selAttrs.size() << " fields selected.\n";
                    break;
                case SQLType::INSERT:
                    std::cout << "Query type: INSERT, ";
                    std::cout << pq.insertFields.size() << " fields selected.\n";
                    std::cout << pq.values.size() << " fields selected.\n";
                    break;
                case SQLType::UPDATE:
                    std::cout << "Query type: UPDATE, ";
                    std::cout << pq.updateAttr.attrName << ": " <<
                    pq.updateAttr.relName<< " fields selected.\n";
                    break;
                case SQLType::DELETE:
                    std::cout << "Query type: DELETE, ";
                    std::cout << pq.deleteTableName << " fields selected.\n";
            }
        } else {
            std::cerr << "Parse failed.\n";
        }
        std::cout << std::endl;
    }
    return 0;
}