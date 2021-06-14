#include "translator.h"
#include "parser.h"
#include <cstdlib>
#include <iostream>

using namespace std;

int quadNo = 1; // 下一条四元式序号，第一条四元式地址为 1
vector<SymbolTable> blocks;
vector<Quad> quads;
// 引用外部文法、属性栈、作用域指针
extern Grammar grammar;
extern stack<Attribute> attrStack;
extern int curScope;

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
    
    Attribute res;	// 归约后产生式左部的属性
	vector<Attribute> attrCache;	// 缓存一下弹出的属性
    Production production = grammar.prods[no];
    Attribute::chain_t Attribute::* pchain;
    SymbolTable* curSymTable = nullptr; // 当前符号表

    if(blocks.size() != 0) {
        curSymTable = &( blocks.back() );
    }

    int rSize = production.right.size();
    // 处理epsilon
    if(production.right[0] != Parser::epsilon) {
        // 属性栈缓存处理
        for(int i = 0; i < rSize; i++) {
            attrCache.push_back(attrStack.top());
            attrStack.pop();
        }
    }

    switch(no) {

    // Program : Block
    case 0: 
        pchain = &Attribute::chain;	// 回填最后一部分
		attrCache[0].backpatch(pchain, quadNo, quads);
        quads.push_back(Quad("End", "_", "_", "_"));
        break;

    // Block : { Decls Stmts }
    case 1:
		res.chain = attrCache[1].chain;
		break;

    // Decls : Decls Decl
    case 2: break;

    // Decls : ε
    case 3: break;

    // Decl : Type id ;
    case 4: 
        res.type = attrCache[2].type;
        Identifier id;
        id.type = attrCache[2].type;
        id.scope = curScope;
        if (curSymTable->insert(attrCache[1].lex, id)) {
			// TODO 赋初值
		} else {
            // 重定义
			string errMsg = "Line " + to_string(res.token.curLine) 
                            + "@<" + to_string(res.token.curStart) + ","
                            + to_string(res.token.curEnd) + ">: ";
			throw runtime_error(errMsg + "multiple definition!"); 
        }
        break;

    // TODO Type : Type [ int_num ]
    case 5: 

        break;

    // Type : int
    case 6: 
        res.type = Identifier::INT;
        break;

    // Type : real
    case 7: 
        res.type = Identifier::REAL;
        break;

    // Type : bool
    case 8: 
        res.type = Identifier::BOOL;
        break;

    // Stmts : Stmts Stmt
    case 9: 

        break; 

    // Stmts : ε
    case 10: break; 

    // Stmt : Var = Bool ;
    case 11: 

        break; 

    // Stmt : if ( Bool ) Stmt
    case 12: 

        break; 
        
    // Stmt : if ( Bool ) Stmt else Stmt
    case 13: 

        break; 
        
    // Stmt : while ( Bool ) Stmt
    case 14: 

        break; 
        
    // Stmt : break ;
    case 15: 

        break; 
        
    // Stmt : Block
    default:
        cout << "Syntax Directed Translation Err at " << no << "!!!" << endl;
        break;
    }
    attrStack.push(res);
}

void printSymbolTable(SymbolTable& table) {
    printf("-------------------------\n");
    printf("|      SymbolTable      |\n");
    printf("-------------------------\n");
    for(auto& var : table.table) {
        cout << var.first << ": " << var.second.value << endl;
    }
}