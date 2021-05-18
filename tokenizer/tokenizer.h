#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "common.h"
#include <string>

using namespace std;

enum TokenType {
    UNKNOWN, // 未知类型
    INT,
    REAL, // 实数
    BOOL,
    TRUE,
    FALSE,
    IF,
    ELIF, // else-if
    ELSE,
    FOR,
    RETURN,
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
    GREATER_EQAUL,
    LESS,
    LESS_EQUAL,
    COMMA, // ,
    SEMICOLON, // ;
    LEFT_BLOCK,
    RIGHT_BLOCK,
    SQ_LEFT_BRACKET, // [
    SQ_RIGHT_BRACKET, // ]
    CIR_LEFT_BRACKET, // (
    CIR_RIGHT_BRACKET, // )
    ERROR
};

struct Token {
    const char* start; // 字符指针，指向token内容的起始位置
    string lex; // 词素(内容本身)
    TokenType type; // Token类型
    uint curLine; // 当前行
    uint length; // 长度
};

char* readFile(const char* path);

#endif