#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "common.h"
#include <string>
#include <vector>

using namespace std;

typedef enum {
    UNKNOWN = 0, // 未知类型
    INT,
    INT_NUM,
    REAL, // 实数
    REAL_NUM,
    BOOL,
    TRUE,
    FALSE,
    IF,
    ELSE,
    WHILE,
    BREAK,
    ID, //标识符
    ADD,
    SUB,
    MUL,
    DIV,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    AND,
    OR,
    NOT,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    COMMA, // ,
    SEMICOLON, // ;
    LEFT_BLOCK,
    RIGHT_BLOCK,
    SQ_LEFT_BRACKET, // [
    SQ_RIGHT_BRACKET, // ]
    CIR_LEFT_BRACKET, // (
    CIR_RIGHT_BRACKET // )
} TokenType;

struct Token {
    string lex; // 词素(内容本身)
    TokenType type; // Token类型
    int curLine; // 当前行
    int start; // 起始指针（全局）
    int end; // 尾指针（全局）
    int curStart;// 起始指针（当前行）
    int curEnd;// 尾指针（当前行）
};

string readFile(const char* path); // 读取文本文件
Token* makeToken(string str, TokenType type); // 字符串转Token
int lexToTokens(string& content, vector<Token>& tokens); // lex转tokens

#endif