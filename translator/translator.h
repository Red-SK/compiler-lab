/**
 * 语法制导翻译器
 **/
#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include "tokenizer.h"
#include <map>

using namespace std;

// 变量（标识符）
struct Identifier {
    enum Type{
		INT = 0,
		REAL,
		BOOL
	}; 
	Type type; // 值类型
	double value; // 值内容
    int scope;   // 作用域(最外层为0)
};

/* 
* [符号表]
* 一个block对应一张表=>作用域
* 编译过程中使用的所有符号表都保存在 vector<SymbolTable> 中
* 每进入一个块 (读入 { )，都要新增一个符号表用来存储该块内的变量信息
* 每退出一个块 (读入 } )，都要删除该块对应的符号表
*/
struct SymbolTable {
    // 通过变量名找到对应的变量
    map<string,Identifier> table;
};

// 四元式
struct Quad {
	string opt;  // 操作
	string lhs;  // 左源操作数
	string rhs;  // 右源操作数
	string dest; // 目标数
	// 构造函数
	Quad():opt(), lhs(), rhs(), dest(){}
	Quad(string o, string l, string r, string d):opt(o), lhs(l), rhs(r), dest(d){}
};

int findIdInfo(string& idName);
bool setIdVal(string& idName, string& num, int type);
/* 语法制导翻译核心 */
void syntaxDirectedTranslation(int productionId);

#endif