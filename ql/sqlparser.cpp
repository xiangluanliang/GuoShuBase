
#include "sqlparser.h"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "ql_error.h"

class SQLParser {
public:
    SQLParser(const std::vector<Token> &tokens)
            : tokens(tokens), pos(0) {}

    RC Parse(ParsedQuery &out) {
        if (match(TokenType::SELECT) == 0) return parseSelect(out);
        else if (match(TokenType::INSERT) == 0) return parseInsert(out);
        else if (match(TokenType::DELETE) == 0) return parseDelete(out);
        else if (match(TokenType::UPDATE) == 0) return parseUpdate(out);
        else if (match(TokenType::CREATE) == 0) {
            if (match(TokenType::TABLE) == 0) return parseCreateTable(out);
//            if (match(TokenType::INDEX) == 0) return parseCreateIndex(out);
        }
        else if (match(TokenType::DROP) == 0) {
            if (match(TokenType::TABLE) == 0) return parseDropTable(out);
//            if (match(TokenType::INDEX) == 0) return parseDropIndex(out);
        }
//        else if (match(TokenType::HELP) == 0) return parseHelp(out);
//        else if (match(TokenType::PRINT) == 0) return parsePrint(out);
//        else if (match(TokenType::LOAD) == 0) return parseLoad(out);
        else {
            return QL_INVALIDQUERY+1;
        }
    }

    static void FreeParsedQuery(ParsedQuery &q) {
        // SELECT
        for (auto &attr : q.selAttrs) {
            if (attr.relName) free(attr.relName);
            if (attr.attrName) free(attr.attrName);
        }

        for (auto rel : q.relations) {
            if (rel) free(rel);
        }

        for (auto &cond : q.conditions) {
            if (cond.lhsAttr.relName) free(cond.lhsAttr.relName);
            if (cond.lhsAttr.attrName) free(cond.lhsAttr.attrName);
            if (cond.bRhsIsAttr) {
                if (cond.rhsAttr.relName) free(cond.rhsAttr.relName);
                if (cond.rhsAttr.attrName) free(cond.rhsAttr.attrName);
            } else {
                if (cond.rhsValue.type == STRING) {
                    free(cond.rhsValue.data);
                } else if (cond.rhsValue.type == INT) {
                    delete static_cast<int*>(cond.rhsValue.data);
                } else if (cond.rhsValue.type == FLOAT) {
                    delete static_cast<float*>(cond.rhsValue.data);
                }
            }
        }

        if (q.groupAttr.relName) free(q.groupAttr.relName);
        if (q.groupAttr.attrName) free(q.groupAttr.attrName);

        for (auto &ra : q.orderAttrs) {
            if (ra.relName) free(ra.relName);
            if (ra.attrName) free(ra.attrName);
        }

        // INSERT
        if (q.insertTableName) free(q.insertTableName);
        for (auto f : q.insertFields) {
            if (f) free(f);
        }
        for (auto &v : q.values) {
            if (v.type == STRING) {
                free(v.data);
            } else if (v.type == INT) {
                delete static_cast<int*>(v.data);
            } else if (v.type == FLOAT) {
                delete static_cast<float*>(v.data);
            }
        }

        // DELETE
        if (q.deleteTableName) free(q.deleteTableName);

        // UPDATE
        if (q.updateTableName) free(q.updateTableName);
        if (q.updateAttr.relName) free(q.updateAttr.relName);
        if (q.updateAttr.attrName) free(q.updateAttr.attrName);
        if (q.updateIsValue) {
            if (q.updateValue.type == STRING) {
                free(q.updateValue.data);
            } else if (q.updateValue.type == INT) {
                delete static_cast<int*>(q.updateValue.data);
            } else if (q.updateValue.type == FLOAT) {
                delete static_cast<float*>(q.updateValue.data);
            }
        } else {
            if (q.updateRhsAttr.relName) free(q.updateRhsAttr.relName);
            if (q.updateRhsAttr.attrName) free(q.updateRhsAttr.attrName);
        }

        // CREATE TABLE
        if (q.createTableName) free(q.createTableName);
        for (auto &a : q.attrList) {
            if (a.attrName) free(a.attrName);
        }

        // DROP TABLE
        if (q.dropTableName) free(q.dropTableName);

        // INDEX
        if (q.indexTableName) free(q.indexTableName);
        if (q.indexAttrName) free(q.indexAttrName);
    }



private:
    const std::vector<Token> &tokens;
    size_t pos;

    Token peek() const {
        return pos < tokens.size() ? tokens[pos] : Token{TokenType::END, ""};
    }

    Token next() {
        return tokens[pos++];
    }

    RC match(TokenType expected) {
        if (peek().type == expected) {
            pos++;
            return 0;
        }
        return QL_INVALIDQUERY+2;
    }

    RC parseSelect(ParsedQuery &q) {
        q.type = SQLType::SELECT;

        if (!parseSelectList(q.selAttrs)) return QL_SELECTERR;
        if (match(TokenType::FROM)) return QL_SELECTERR;
        if (!parseTableList(q.relations)) return QL_SELECTERR;

        if (!match(TokenType::WHERE)) {
            if (!parseConditionList(q.conditions)) return QL_SELECTERR;
        }

        if (!match(TokenType::GROUP)) {
            if (match(TokenType::BY)) return QL_SELECTERR;
            if (!parseRelAttr(q.groupAttr)) return QL_SELECTERR;
            q.hasGroupBy = true;
        }

        if (!match(TokenType::ORDER)) {
            if (match(TokenType::BY)) return QL_SELECTERR;
            do {
                RelAttr ra;
                if (!parseRelAttr(ra)) return QL_SELECTERR;
                q.orderAttrs.push_back(ra);
            } while (!match(TokenType::COMMA));
            q.hasOrderBy = true;
        }

        return match(TokenType::SEMICOLON);
    }

    RC parseInsert(ParsedQuery &q) {
        q.type = SQLType::INSERT;

        if (match(TokenType::INTO)) return QL_INSERTERR;
        q.insertTableName = AllocCopyString(next().text);

        if (!match(TokenType::LPAREN)) {
            do {
                q.insertFields.push_back(AllocCopyString(next().text));
            } while (match(TokenType::COMMA));
            if (!match(TokenType::RPAREN)) return QL_INSERTERR;
        }

        if (match(TokenType::VALUES)) return QL_INSERTERR;
        if (match(TokenType::LPAREN)) return QL_INSERTERR;

        do {
            Value v;
            if (peek().type == TokenType::INT_LITERAL) {
                v.type = INT;
                int* num = new int(std::stoi(next().text));
                v.data = num;
            } else if (peek().type == TokenType::STRING_LITERAL) {
                v.type = STRING;
                v.data = AllocCopyString(next().text);
            } else {
                return QL_INSERTERR;
            }
            q.values.push_back(v);
        } while (!match(TokenType::COMMA));

        return match(TokenType::RPAREN) || match(TokenType::SEMICOLON);
    }

    RC parseDelete(ParsedQuery &q) {
        q.type = SQLType::DELETE;

        if (match(TokenType::FROM)) return QL_DELETEERR;
        q.deleteTableName = AllocCopyString(next().text);

        if (!match(TokenType::WHERE)) {
            if (!parseConditionList(q.conditions)) return QL_DELETEERR;
        }

        return match(TokenType::SEMICOLON);
    }

    RC parseUpdate(ParsedQuery &q) {
        q.type = SQLType::UPDATE;

        q.updateTableName = AllocCopyString(next().text);
        if (match(TokenType::SET)) return QL_UPDATEERR;

        if (!parseRelAttr(q.updateAttr)) return QL_UPDATEERR;
        if (match(TokenType::EQ)) return QL_UPDATEERR;

        if (peek().type == TokenType::IDENTIFIER) {
            q.updateIsValue = false;
            if (!parseRelAttr(q.updateRhsAttr)) return QL_UPDATEERR;
        } else if (peek().type == TokenType::INT_LITERAL) {
            q.updateIsValue = true;
            q.updateValue.type = INT;
            int* num = new int(std::stoi(next().text));
            q.updateValue.data = num;
        } else if (peek().type == TokenType::STRING_LITERAL) {
            q.updateIsValue = true;
            q.updateValue.type = STRING;
            q.updateValue.data = AllocCopyString(next().text);
        } else {
            return QL_UPDATEERR;
        }

        if (!match(TokenType::WHERE)) {
            if (!parseConditionList(q.conditions)) return QL_UPDATEERR;
        }

        return match(TokenType::SEMICOLON);
    }

    RC parseCreateTable(ParsedQuery &q) {
        q.type = SQLType::CREATE_TABLE;

        if (peek().type != TokenType::IDENTIFIER) return QL_CREATETABLEERR;
        q.createTableName = AllocCopyString(next().text);

        if (match(TokenType::LPAREN)) return QL_CREATETABLEERR;

        do {
            AttrInfo attr;
            if (peek().type != TokenType::IDENTIFIER) return QL_CREATETABLEERR;
            attr.attrName = AllocCopyString(next().text);

            if (peek().type != TokenType::IDENTIFIER) return QL_CREATETABLEERR;
            std::string typeStr = next().text;

            if (typeStr == "INT") {
                attr.attrType = INT;
                attr.attrLength = sizeof(int);
            } else if (typeStr == "FLOAT") {
                attr.attrType = FLOAT;
                attr.attrLength = sizeof(float);
            } else if (typeStr == "STRING") {
                if (match(TokenType::LPAREN)) return QL_CREATETABLEERR;
                if (peek().type != TokenType::INT_LITERAL) return QL_CREATETABLEERR;
                attr.attrLength = std::stoi(next().text);
                if (match(TokenType::RPAREN)) return QL_CREATETABLEERR;
                attr.attrType = STRING;
            } else {
                return QL_CREATETABLEERR;
            }

            q.attrList.push_back(attr);
        } while (!match(TokenType::COMMA));

        return match(TokenType::RPAREN) || match(TokenType::SEMICOLON);
    }

    RC parseDropTable(ParsedQuery &q) {
        q.type = SQLType::DROP_TABLE;
        if (peek().type != TokenType::IDENTIFIER) return QL_DROPTABLEERR;
        q.dropTableName = AllocCopyString(next().text);
        return match(TokenType::SEMICOLON);
    }

    bool parseSelectList(std::vector<AggRelAttr> &attrs) {
        do {
            AggRelAttr attr;
            attr.func = NO_F;
            attr.relName = nullptr;
            attr.attrName = nullptr;

            if (peek().type == TokenType::IDENTIFIER &&
                (peek().text == "COUNT" || peek().text == "SUM" || peek().text == "AVG" ||
                 peek().text == "MIN" || peek().text == "MAX")) {
                // 聚合函数
                attr.func = strToAggFun(next().text);
                if (match(TokenType::LPAREN)) return false;
                if (peek().type == TokenType::STAR) {
                    next();
                    attr.attrName = AllocCopyString("*");
                } else {
                    RelAttr ra;
                    if (!parseRelAttr(ra)) return false;
                    attr.relName = ra.relName ? AllocCopyString(ra.relName) : nullptr;
                    attr.attrName = ra.attrName ? AllocCopyString(ra.attrName) : nullptr;
                }
                if (match(TokenType::RPAREN)) return false;
            } else if (peek().type == TokenType::STAR) {
                // SELECT *
                next();
                attr.attrName = AllocCopyString("*");
                attr.relName = nullptr;
                attr.func = NO_F;
            } else {
                // 普通字段或 rel.attr
                RelAttr ra;
                if (!parseRelAttr(ra)) return false;
                attr.func = NO_F;
                attr.relName = ra.relName ? AllocCopyString(ra.relName) : nullptr;
                attr.attrName = ra.attrName ? AllocCopyString(ra.attrName) : nullptr;
            }

            attrs.push_back(attr);
        } while (!match(TokenType::COMMA));

        return true;
    }


    bool parseTableList(std::vector<char*> &tables) {
        do {
            tables.push_back(AllocCopyString(next().text));
        } while (!match(TokenType::COMMA));
        return true;
    }

    bool parseRelAttr(RelAttr &attr) {
        // 解析 rel.attr 或 attr
        if (peek().type != TokenType::IDENTIFIER) return false;
        std::string first = next().text;

        if (!match(TokenType::DOT)) {
            // rel.attr 形式
            if (peek().type != TokenType::IDENTIFIER) return false;
            std::string second = next().text;
            attr.relName = AllocCopyString(first);
            attr.attrName = AllocCopyString(second);
        } else {
            // attr 形式
            attr.relName = nullptr;
            attr.attrName = AllocCopyString(first);
        }
        return true;
    }


    bool parseConditionList(std::vector<Condition> &conds) {
        do {
            Condition cond;

            // lhsAttr
            if (!parseRelAttr(cond.lhsAttr)) return false;

            // 比较操作符
            TokenType opToken = peek().type;
            cond.op = tokenToCompOp(opToken);
            if (cond.op == NO_OP) return false;
            next(); // consume operator

            // rhs 可能是 attr 或 literal
            if (peek().type == TokenType::IDENTIFIER) {
                cond.bRhsIsAttr = 1;
                if (!parseRelAttr(cond.rhsAttr)) return false;
            } else if (peek().type == TokenType::INT_LITERAL) {
                cond.bRhsIsAttr = 0;
                cond.rhsValue.type = INT;
                int* val = new int(std::stoi(next().text));
                cond.rhsValue.data = val;
            } else if (peek().type == TokenType::FLOAT_LITERAL) {
                cond.bRhsIsAttr = 0;
                cond.rhsValue.type = FLOAT;
                float* val = new float(std::stof(next().text));
                cond.rhsValue.data = val;
            } else if (peek().type == TokenType::STRING_LITERAL) {
                cond.bRhsIsAttr = 0;
                cond.rhsValue.type = STRING;
                cond.rhsValue.data = AllocCopyString(next().text);
            } else {
                return false;
            }

            conds.push_back(cond);
        } while (!match(TokenType::AND));

        return true;
    }


    AggFun strToAggFun(const std::string &s) {
        if (s == "COUNT") return COUNT_F;
        if (s == "SUM") return SUM_F;
        if (s == "AVG") return AVG_F;
        if (s == "MAX") return MAX_F;
        if (s == "MIN") return MIN_F;
        return NO_F;
    }

    CompOp tokenToCompOp(TokenType t) {
        switch (t) {
            case TokenType::EQ: return EQ_OP;
            case TokenType::NE: return NE_OP;
            case TokenType::LT: return LT_OP;
            case TokenType::GT: return GT_OP;
            case TokenType::LE: return LE_OP;
            case TokenType::GE: return GE_OP;
            default: return NO_OP;
        }
    }


    // Helper functions
    static char* AllocCopyString(const std::string& s) {
        char* p = strdup(s.c_str());
        if (!p) throw std::bad_alloc();
        return p;
    }

};

RC ParseSQL(const std::vector<Token> &tokens, ParsedQuery &out) {
    RC rc;
    SQLParser parser(tokens);
    if ((rc = parser.Parse(out))) {
        SQLParser::FreeParsedQuery(out);
        return rc;
    }
    return rc;
}
