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
        Factor	 
    } NonTerminator;

    // 动作
    typedef enum {
        Accept = 0, // 接受
        Shift, // 移进
        Reduce // 规约
    } Action;
}

/***  
 *  Grammar[Program]
 *  LR(1)
 *  ε: epsilon
 * 
    Program   → Block
    Block     → { Decls Stmts }
    Decls     → Decls Decl
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
    bool operator==(Production& rhs) const {
        if (left != rhs.left)
            return false;
        for (int i = 0; i < right.size(); i++) {
            if (i >= rhs.right.size())
                return false;
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
    Parser::Terminator next;
};

// LR1项目集
struct LR1ItemSet {
    vector<LR1Item> items;
};

// LR1项目集规范族
struct CanonicalCollection {
    // 项目集集合
    vector<LR1ItemSet> itemSets;
    // 保存DFA的图
    // []为当前状态序号
    // first为转移到的状态序号
    // second是经什么转移（即吃掉的符号）
    vector<pair<int,int>> g[100];
};

// 文法结构体
struct Grammar {
    vector<string> T;    // 终结符
    vector<string> N; // 非终结符
    vector<Production> prods;        // 产生式
};

// FIRST集
typedef map<Parser::NonTerminator,set<Parser::Terminator>> FIRST;
// FOLLOW集
typedef map<Parser::NonTerminator,set<Parser::Terminator>> FOLLOW;

void test4parser();

#endif