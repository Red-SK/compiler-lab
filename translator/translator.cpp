#include "translator.h"
#include "parser.h"
#include <cstdlib>

vector<SymbolTable> blocks;
vector<Quad> quads;
// 引用外部文法
extern Grammar grammar;

// 找到变量所在作用域（从近到远），返回scope，失败返回-1
int findIdScope(string& idName) {
    int size = blocks.size();
    for(int i = size - 1; i >= 0; i--) {
        // 对应的变量存在
        if(blocks[i].table.find(idName) != blocks[i].table.end()) {
            return i;
        }
    }
    // 找不到
    return -1;
}

// 变量赋值（整数、实数、布尔）
bool setIdVal(string& idName, string& num, int type) {
    int size = blocks.size();
    for(int i = size - 1; i >= 0; i--) {
        // 对应的变量存在
        if(blocks[i].table.find(idName) != blocks[i].table.end()) {
            // 整数
            if(type == TokenType::INT_NUM) {
                blocks[i].table[idName].value = (double) atoi(num.c_str());
                return true;
            }
            // 实数
            else if(type == TokenType::REAL_NUM) {
                blocks[i].table[idName].value = atof(num.c_str());
                return true;
            }
            // 布尔
            else if(type == TokenType::TRUE) {
                blocks[i].table[idName].value = (double) 1;
                return true;
            }
            else if(type == TokenType::FALSE) {
                blocks[i].table[idName].value = (double) 0;
                return true;
            }
            // 其他=>错误
            else {
                return false;
            }
        }
    }
    // 找不到
    return false;
}

/* 
* 语法制导翻译核心 
* 输入：产生式序号
*/
void syntaxDirectedTranslation(int no) {
    Production production = grammar.prods[no];
    SymbolTable* curSymTable = nullptr;

    if(blocks.size() != 0) {
        curSymTable = &( blocks.back() );
    }

    // TODO 处理epsilon

    // TODO 属性栈缓存

    switch(no) {
    //Program : Block
    case 0: 
        // TODO 回填
        quads.push_back(Quad("End", "_", "_", "_"));
        break;
    }
}