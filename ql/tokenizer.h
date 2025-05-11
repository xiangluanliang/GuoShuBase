//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/10 21:37
// @Project: GuoShuBase
//
//

#ifndef TOKENIZER_H
#define TOKENIZER_H
#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>

enum class TokenType {
    SELECT, INSERT, DELETE, UPDATE,
    FROM, WHERE, INTO, VALUES, GROUP, BY, ORDER, HAVING, SET,
    AND, OR, NOT,
    STAR, COMMA, SEMICOLON,DOT,
    EQ, NE, LT, GT, LE, GE,
    LPAREN, RPAREN,
    IDENTIFIER,
    STRING_LITERAL,
    INT_LITERAL,
    END,
    INVALID
};

struct Token {
    TokenType type;
    std::string text;
};

std::vector<Token> Tokenize(const std::string &sql);

std::string TokenTypeToString(TokenType type);


#endif //TOKENIZER_H
