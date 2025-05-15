//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/10 21:37
// @Project: GuoShuBase
//
//

#include "tokenizer.h"
#include <sstream>
#include <iostream>

static std::unordered_map<std::string, TokenType> keywords = {
        {"SELECT", TokenType::SELECT},
        {"INSERT", TokenType::INSERT},
        {"DELETE", TokenType::DELETE},
        {"UPDATE", TokenType::UPDATE},
        {"FROM", TokenType::FROM},
        {"WHERE", TokenType::WHERE},
        {"INTO", TokenType::INTO},
        {"VALUES", TokenType::VALUES},
        {"AND", TokenType::AND},
        {"OR", TokenType::OR},
        {"NOT", TokenType::NOT},
        {"GROUP", TokenType::GROUP},
        {"BY", TokenType::BY},
        {"ORDER", TokenType::ORDER},
        {"HAVING", TokenType::HAVING},
        {"SET", TokenType::SET},
        {"CREATE", TokenType::CREATE},
        {"DROP", TokenType::DROP},
        {"INDEX", TokenType::INDEX},
        {"ON", TokenType::ON},
        {"TABLE", TokenType::TABLE},
        {"ALTER", TokenType::ALTER}
};

std::string TokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::SELECT:
            return "SELECT";
        case TokenType::INSERT:
            return "INSERT";
        case TokenType::DELETE:
            return "DELETE";
        case TokenType::UPDATE:
            return "UPDATE";
        case TokenType::FROM:
            return "FROM";
        case TokenType::WHERE:
            return "WHERE";
        case TokenType::INTO:
            return "INTO";
        case TokenType::VALUES:
            return "VALUES";
        case TokenType::ALTER:
            return "ALTER";
        case TokenType::TABLE:
            return "TABLE";
        case TokenType::CREATE:
            return "CREATE";
        case TokenType::ADD:
            return "ADD";
        case TokenType::DROP:
            return "DROP";
        case TokenType::INDEX:
            return "INDEX";
        case TokenType::AND:
            return "AND";
        case TokenType::OR:
            return "OR";
        case TokenType::STAR:
            return "*";
        case TokenType::COMMA:
            return ",";
        case TokenType::SEMICOLON:
            return ";";
        case TokenType::EQ:
            return "=";
        case TokenType::NE:
            return "<>";
        case TokenType::LT:
            return "<";
        case TokenType::GT:
            return ">";
        case TokenType::LE:
            return "<=";
        case TokenType::GE:
            return ">=";
        case TokenType::LPAREN:
            return "(";
        case TokenType::RPAREN:
            return ")";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::STRING_LITERAL:
            return "STRING";
        case TokenType::INT_LITERAL:
            return "INT";
        case TokenType::GROUP:
            return "GROUP";
        case TokenType::BY:
            return "BY";
        case TokenType::ORDER:
            return "ORDER";
        case TokenType::HAVING:
            return "HAVING";
        case TokenType::SET:
            return "SET";
        case TokenType::NOT:
            return "NOT";
        case TokenType::END:
            return "END";
        default:
            return "INVALID";
    }
}

std::vector<Token> Tokenize(const std::string &sql) {
    std::vector<Token> tokens;
    size_t i = 0;
    size_t len = sql.length();

    auto skipWhitespace = [&]() {
        while (i < len && isspace(sql[i])) i++;
    };

    skipWhitespace();
    while (i < len) {
        if (isalpha(sql[i]) || sql[i] == '_') {
            std::string word;
            while (i < len && (isalnum(sql[i]) || sql[i] == '_')) {
                word += sql[i++];
            }

            // 保留原始 word 供标识符使用
            std::string keywordKey;
            for (char c: word)
                keywordKey += std::toupper(c);

            if (keywords.count(keywordKey)) {
                tokens.push_back({keywords[keywordKey], keywordKey});  // 关键字转大写
            } else {
                tokens.push_back({TokenType::IDENTIFIER, word});  // 用户标识符保留原样
            }
        } else if (isdigit(sql[i])) {
            std::string number;
            bool isFloat = false;
            // 整数部分
            while (i < len && isdigit(sql[i]))
                number += sql[i++];

            // 小数点部分
            if (i < len && sql[i] == '.') {
                isFloat = true;
                number += sql[i++];  // 加入点
                // 小数点后至少要有一个数字
                if (i < len && isdigit(sql[i])) {
                    while (i < len && isdigit(sql[i])) {
                        number += sql[i++];
                    }
                } else {
                    // 错误：点后没有数字 → 回退为整数
                    isFloat = false;
                    i--; // 回退，把点还回去
                    number.pop_back();
                }
            }
            if (isFloat)
                tokens.push_back({TokenType::FLOAT_LITERAL, number});
            else
                tokens.push_back({TokenType::INT_LITERAL, number});

        } else if (sql[i] == '\'') {
            std::string literal;
            i++; // skip '
            while (i < len && sql[i] != '\'') {
                literal += sql[i++];
            }
            i++; // skip closing '
            tokens.push_back({TokenType::STRING_LITERAL, literal});
        } else {
            switch (sql[i]) {
                case '*':
                    tokens.push_back({TokenType::STAR, "*"});
                    i++;
                    break;
                case '.':
                    tokens.push_back({TokenType::DOT, "."});
                    i++;
                    break;
                case ',':
                    tokens.push_back({TokenType::COMMA, ","});
                    i++;
                    break;
                case ';':
                    tokens.push_back({TokenType::SEMICOLON, ";"});
                    i++;
                    break;
                case '=':
                    tokens.push_back({TokenType::EQ, "="});
                    i++;
                    break;
                case '<':
                    if (i + 1 < len && sql[i + 1] == '=') {
                        tokens.push_back({TokenType::LE, "<="});
                        i += 2;
                    } else if (i + 1 < len && sql[i + 1] == '>') {
                        tokens.push_back({TokenType::NE, "<>"});
                        i += 2;
                    } else {
                        tokens.push_back({TokenType::LT, "<"});
                        i++;
                    }
                    break;
                case '>':
                    if (i + 1 < len && sql[i + 1] == '=') {
                        tokens.push_back({TokenType::GE, ">="});
                        i += 2;
                    } else {
                        tokens.push_back({TokenType::GT, ">"});
                        i++;
                    }
                    break;
                case '(':
                    tokens.push_back({TokenType::LPAREN, "("});
                    i++;
                    break;
                case ')':
                    tokens.push_back({TokenType::RPAREN, ")"});
                    i++;
                    break;
                default:
                    std::cerr << "Unknown token: " << sql[i] << "\n";
                    tokens.push_back({TokenType::INVALID, std::string(1, sql[i])});
                    i++;
            }
        }
        skipWhitespace();
    }

    tokens.push_back({TokenType::END, ""});
    return tokens;
}
