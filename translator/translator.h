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
	using chain_t = std::unordered_set<size_t>;	// 用于回填的四元式地址链

	Attribute() = default;
	~Attribute() = default;

	chain_t chain;	// 和拉链回填有关的属性
	chain_t TC;
	chain_t FC;
	int quad;

	string lex;				// 词素
	Identifier::Type type;	// 类型
	double value;			// 变量或表达式的值
	string tempIdName;		// 临时变量名
	Token  token;			// Token信息

	// 回填
	void backpatch(chain_t Attribute::* pchain, int n, std::vector<Quad> & quads) {
		for (auto& i : this->*pchain) {
			quads[i].dest = to_string(n);
		}
	}
	// 合并
	void merge(chain_t Attribute::*pchain, const chain_t& chain1, const chain_t& chain2) {
		(this->*pchain).insert(chain1.begin(), chain1.end());
		(this->*pchain).insert(chain2.begin(), chain2.end());
	}
};

int findIdInfo(string& idName);
bool setIdVal(string& idName, string& num, int type);
/* 语法制导翻译核心 */
void syntaxDirectedTranslation(int productionId);
void printSymbolTable(SymbolTable& table);

#endif