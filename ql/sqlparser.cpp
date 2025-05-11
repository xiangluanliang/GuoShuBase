
#include "sqlparser.h"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cstdlib>

class SQLParser {
public:
    SQLParser(const std::vector<Token> &tokens)
            : tokens(tokens), pos(0) {}

    bool Parse(ParsedQuery &out) {
        try {
            if (match(TokenType::SELECT)) {
                return parseSelect(out);
            } else if (match(TokenType::INSERT)) {
                return parseInsert(out);
            } else if (match(TokenType::DELETE)) {
                return parseDelete(out);
            } else if (match(TokenType::UPDATE)) {
                return parseUpdate(out);
            }
            return false;
        } catch (...) {
            FreeParsedQuery(out);
            throw;
        }
    }

    static void FreeParsedQuery(ParsedQuery& q) {
        // Free SELECT parts
        for (auto& attr : q.selAttrs) FreeAggRelAttr(attr);
        for (char* rel : q.relations) free(rel);
        for (auto& cond : q.conditions) FreeCondition(cond);
        FreeRelAttr(q.groupAttr);
        for (auto& attr : q.orderAttrs) FreeRelAttr(attr);

        // Free INSERT parts
        free(q.insertTableName);
        for (char* field : q.insertFields) free(field);
        for (auto& val : q.values) FreeValue(val);

        // Free DELETE parts
        free(q.deleteTableName);

        // Free UPDATE parts
        free(q.updateTableName);
        FreeRelAttr(q.updateAttr);
        FreeValue(q.updateValue);
        FreeRelAttr(q.updateRhsAttr);
    }

private:
    const std::vector<Token> &tokens;
    size_t pos;

    // Helper functions
    static char* AllocCopyString(const std::string& s) {
        char* p = strdup(s.c_str());
        if (!p) throw std::bad_alloc();
        return p;
    }

    static void FreeValue(Value& v) {
        if (v.type == STRING) {
            free(v.data);
        } else if (v.type == INT) {
            delete static_cast<int*>(v.data);
        }
        v.data = nullptr;
    }

    static void FreeRelAttr(RelAttr& attr) {
        free(attr.relName);
        free(attr.attrName);
        attr.relName = attr.attrName = nullptr;
    }

    static void FreeAggRelAttr(AggRelAttr& attr) {
        free(attr.relName);
        free(attr.attrName);
        attr.relName = attr.attrName = nullptr;
    }

    static void FreeCondition(Condition& cond) {
        FreeRelAttr(cond.lhsAttr);
        if (cond.bRhsIsAttr) {
            FreeRelAttr(cond.rhsAttr);
        } else {
            FreeValue(cond.rhsValue);
        }
    }

    Token peek() const {
        return pos < tokens.size() ? tokens[pos] : Token{TokenType::END, ""};
    }

    Token next() {
        return tokens[pos++];
    }

    bool match(TokenType expected) {
        if (peek().type == expected) {
            pos++;
            return true;
        }
        return false;
    }

    bool parseSelect(ParsedQuery &q) {
        q.type = SQLType::SELECT;

        if (!parseSelectList(q.selAttrs)) return false;
        if (!match(TokenType::FROM)) return false;
        if (!parseTableList(q.relations)) return false;

        if (match(TokenType::WHERE)) {
            if (!parseConditionList(q.conditions)) return false;
        }

        if (match(TokenType::GROUP)) {
            if (!match(TokenType::BY)) return false;
            if (!parseRelAttr(q.groupAttr)) return false;
            q.hasGroupBy = true;
        }

        if (match(TokenType::ORDER)) {
            if (!match(TokenType::BY)) return false;
            do {
                RelAttr ra;
                if (!parseRelAttr(ra)) return false;
                q.orderAttrs.push_back(ra);
            } while (match(TokenType::COMMA));
            q.hasOrderBy = true;
        }

        return match(TokenType::SEMICOLON);
    }

    bool parseInsert(ParsedQuery &q) {
        q.type = SQLType::INSERT;

        if (!match(TokenType::INTO)) return false;
        q.insertTableName = AllocCopyString(next().text);

        if (match(TokenType::LPAREN)) {
            do {
                q.insertFields.push_back(AllocCopyString(next().text));
            } while (match(TokenType::COMMA));
            if (!match(TokenType::RPAREN)) return false;
        }

        if (!match(TokenType::VALUES)) return false;
        if (!match(TokenType::LPAREN)) return false;

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
                return false;
            }
            q.values.push_back(v);
        } while (match(TokenType::COMMA));

        return match(TokenType::RPAREN) && match(TokenType::SEMICOLON);
    }

    bool parseDelete(ParsedQuery &q) {
        q.type = SQLType::DELETE;

        if (!match(TokenType::FROM)) return false;
        q.deleteTableName = AllocCopyString(next().text);

        if (match(TokenType::WHERE)) {
            if (!parseConditionList(q.conditions)) return false;
        }

        return match(TokenType::SEMICOLON);
    }

    bool parseUpdate(ParsedQuery &q) {
        q.type = SQLType::UPDATE;

        q.updateTableName = AllocCopyString(next().text);
        if (!match(TokenType::SET)) return false;

        if (!parseRelAttr(q.updateAttr)) return false;
        if (!match(TokenType::EQ)) return false;

        if (peek().type == TokenType::IDENTIFIER) {
            q.updateIsValue = false;
            if (!parseRelAttr(q.updateRhsAttr)) return false;
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
            return false;
        }

        if (match(TokenType::WHERE)) {
            if (!parseConditionList(q.conditions)) return false;
        }

        return match(TokenType::SEMICOLON);
    }

    bool parseSelectList(std::vector<AggRelAttr> &attrs) {
        do {
            AggRelAttr attr;
            attr.func = NO_F;
            attr.relName = nullptr;

            if (peek().type == TokenType::IDENTIFIER) {
                attr.attrName = AllocCopyString(next().text);
            } else if (peek().type == TokenType::STAR) {
                next();
                attr.attrName = AllocCopyString("*");
            } else if (peek().type == TokenType::IDENTIFIER &&
                       (peek().text == "COUNT" || peek().text == "SUM" || peek().text == "AVG")) {
                attr.func = strToAggFun(next().text);
                if (!match(TokenType::LPAREN)) return false;
                if (peek().type == TokenType::STAR) {
                    next();
                    attr.attrName = AllocCopyString("*");
                } else {
                    attr.attrName = AllocCopyString(next().text);
                }
                if (!match(TokenType::RPAREN)) return false;
            } else {
                return false;
            }
            attrs.push_back(attr);
        } while (match(TokenType::COMMA));

        return true;
    }

    bool parseTableList(std::vector<char*> &tables) {
        do {
            tables.push_back(AllocCopyString(next().text));
        } while (match(TokenType::COMMA));
        return true;
    }

    bool parseRelAttr(RelAttr &attr) {
        attr.relName = nullptr;
        attr.attrName = AllocCopyString(next().text);
        return true;
    }

    bool parseConditionList(std::vector<Condition> &conds) {
        do {
            Condition cond;
            if (!parseRelAttr(cond.lhsAttr)) return false;

            TokenType opToken = peek().type;
            cond.op = tokenToCompOp(opToken);
            if (cond.op == NO_OP) return false;
            next();

            if (peek().type == TokenType::IDENTIFIER) {
                cond.bRhsIsAttr = true;
                if (!parseRelAttr(cond.rhsAttr)) return false;
            } else if (peek().type == TokenType::INT_LITERAL) {
                cond.bRhsIsAttr = false;
                cond.rhsValue.type = INT;
                int* num = new int(std::stoi(next().text));
                cond.rhsValue.data = num;
            } else if (peek().type == TokenType::STRING_LITERAL) {
                cond.bRhsIsAttr = false;
                cond.rhsValue.type = STRING;
                cond.rhsValue.data = AllocCopyString(next().text);
            } else {
                return false;
            }
            conds.push_back(cond);
        } while (match(TokenType::AND));

        return true;
    }

    AggFun strToAggFun(const std::string &s) {
        if (s == "COUNT") return COUNT_F;
        if (s == "SUM") return SUM_F;
        if (s == "AVG") return AVG_F;
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
};

bool ParseSQL(const std::vector<Token> &tokens, ParsedQuery &out) {
    SQLParser parser(tokens);
    bool success = parser.Parse(out);
    if (!success) {
        SQLParser::FreeParsedQuery(out);
    }
    return success;
}
