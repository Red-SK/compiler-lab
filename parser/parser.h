/**
 * 语法分析器
 **/
#ifndef _PARSER_H_
#define _PARSER_H_

#include "common.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>

using namespace std;

namespace Parser {

    // 终结符
    typedef enum {
        epsilon = 0, // 空
        int_type,
        int_num,
        real_type,
        real_num,
        bool_type,
        true_val,
        false_val,
        if_kw,
        else_kw,
        while_kw,
        break_kw,
        id, //标识符
        add,
        sub,
        mul,
        div,
        assign,
        equal,
        not_equal,
        and_rel,
        or_rel,
        not_rel,
        greater,
        greater_equal,
        less,
        less_equal,
        comma, // ,
        semicolon, // ;
        left_block,
        right_block,
        sq_left_bracket,  // [
        sq_right_bracket, // ]
        cir_left_bracket, // (
        cir_right_bracket,// )
        eof // $
    } Terminator;

    // 非终结符
    typedef enum {
        // 从100开始，便于区别终结符
        Program = 100,
        Block,
        Decls,
        Decl,    
        Type,    
        Stmts,     
        Stmt,    
        Var,    
        Bool,    
        Join,    
        Equality, 
        Rel,
        Expr,     
        Term,    
        Unary,	 
        Factor,
        HN,
        M
    } NonTerminator;

    // 动作
    typedef enum {
        Accept = 0, // 接受
        Shift, // 移进
        Reduce, // 规约
        Error // 错误，拒绝
    } Action;
}

/***  
 *  Grammar[Program]
 *  LR(1)
 *  ε: epsilon
 *  not the latest!!!
 * 
    Program   → Block
    Block     → { Decls Stmts }
    Decls     → Decls Decl | ε
    Decl      → Type id;
    Type      → Type[int_num] | Type[real_num] | int | real | bool
    Stmts     → Stmts Stmt | ε
    Stmt      → Var=Bool;
              | if(Bool) Stmt
              | if(Bool) Stmt else Stmt
              | while(Bool) Stmt
              | break;
              | Block
    Var      → Var[num] | id
    Bool     → Bool||Join | Join
    Join     → Join&&Equality | Equality
    Equality → Equality==Rel | Equality!=Rel | Rel
    Rel      → Expr<Expr | Expr<=Expr | Expr>=Expr | Expr>Expr | Expr
    Expr     → Expr+Term | Expr-Term | Term
    Term     → Term*Unary | Term/Unary | Unary
    Unary	 → !Unary | -Unary | Factor
    Factor	 → (Bool) | Var | int_num | real_num | true | false
***/

// 产生式结构体，左部符号和右部符号串
struct Production {
    int left; // 产生式左部
    vector<int> right;          // 产生式右部
    bool operator==(const Production& rhs) {
        if (left != rhs.left)
            return false;
        if(rhs.right.size() != right.size())
            return false;
        for (int i = 0; i < right.size(); i++) {
            if (right[i] != rhs.right[i])
                return false;
        }
        return true;
    }
};

// LR1项目
struct LR1Item {
    Production production;
    // 点的位置
    int location;
    // 向前看符号
    int next;
    bool operator==(const LR1Item& p) {
        if(location != p.location) return false;
        if(next != p.next) return false;
        if(!(production == p.production)) return false;
        return true;
    }
    bool operator<(const LR1Item& p) {
        if(location != p.location) return true;
        if(next != p.next) return true;
        if(!(production == p.production)) return true;
        return false;
    }
};

// LR1项目集
struct LR1ItemSet {
    vector<LR1Item> items;
    // 重载不等于
    bool operator!=(const LR1ItemSet& s) {
        if(items.size() != s.items.size()) return true;
        int len = items.size();
        for(int i = 0; i < len; i++) {
            bool f = false;
            for(int j = 0; j < len; j++) {
                if(items[i] == s.items[j]) {
                    f = true;
                }
            }
            if(!f) return true;
        }
        return false;
    }
};

// LR1项目集规范族
struct CanonicalCollection {
    // 项目集集合
    vector<LR1ItemSet> itemSets;
    // 保存DFA的图
    // []为当前状态序号
    // first是经什么转移（即吃掉的符号）
    // second为转移到的状态序号
    vector<pair<int,int>> g[1000];
};

// 文法结构体
struct Grammar {
    vector<int> T;    // 终结符
    vector<int> N; // 非终结符
    vector<Production> prods;        // 产生式
};

void getNullableSet();
void getFirstSet();
void closure(LR1ItemSet& items);
void go(LR1ItemSet& src, int symbol, LR1ItemSet& dst);
void DFA();
void buildPredictTable();
void syntaxParser();
void test4Parser();

#endif