/**
 * 语法制导翻译器
 **/
#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include "tokenizer.h"
#include <map>
#include <vector>
#include <unordered_set>

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

	SymbolTable() = default;

    // 通过变量名找到对应的变量
    map<string,Identifier> table;

	// 插入
	bool insert(string& name, Identifier& id){
		if(table.find(name) != table.end()) {
			return false;
		} else {
			table[name] = id;
			return true;
		}
	}
	// 移除
	void remove(string& name) {
		table.erase(name);
	}
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

// 语法单元属性
struct Attribute {

	Attribute() = default;
	~Attribute() = default;

	unordered_set<int> TC;		// 该属性为真时的跳转指令链
	unordered_set<int> FC;		// 该属性为假时的跳转指令链
	unordered_set<int> NC;		// 跳转到该代码后的跳转指令链
	int nextInstr;				// 下一条指令位置

	string lex;				// 词素
	Identifier::Type type;	// 类型
	double value;			// 变量或表达式的值
	string tempIdName;		// 临时变量名
	Token  token;			// Token信息

};

int findIdScope(string& idName);
bool setIdVal(string& idName, double value);
/* 语法制导翻译核心 */
void syntaxDirectedTranslation(int productionId);
void printSymbolTable(SymbolTable& table);
void printQuads(vector<Quad>& quads);
void backpatch(unordered_set<int>& pchain, int i, vector<Quad>& quads);
unordered_set<int> merge(const unordered_set<int>& chain1, const unordered_set<int>& chain2);

#endif