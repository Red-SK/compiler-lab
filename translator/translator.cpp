#include "translator.h"
#include "parser.h"
#include "common.h"
#include <cstdlib>
#include <iostream>

using namespace std;

int quadNo = 1; // 下一条四元式序号，第一条四元式地址为 1
int tmpIdx = 0; // 分配给临时变量的序号
vector<SymbolTable> blocks;
vector<Quad> quads;
// 引用外部文法、属性栈、作用域指针
extern Grammar grammar;
extern stack<Attribute> attrStack;
extern int curScope;

// 错误信息
string makeErrMsg(Token& token) {
    return "Line "  + to_string(token.curLine) 
                    + "@<" + to_string(token.curStart) + ","
                    + to_string(token.curEnd) + ">: ";
}

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
bool setIdVal(string& idName, double value) {
    int size = blocks.size();
    for(int i = size - 1; i >= 0; i--) {
        // 对应的变量存在
        if(blocks[i].table.find(idName) != blocks[i].table.end()) {
            blocks[i].table[idName].value = value;
            return true;
        }
    }
    // 找不到
    return false;
}

// 打印集合
static void printSet(const unordered_set<int>& s) {
    cout << "{ ";
    for(int num : s) {
        cout << num << " ";
    }
    cout << "}" << endl;
}

// 回填
void backpatch(unordered_set<int>& pchain, int num, vector<Quad>& quads) {
    for (auto& i : pchain) {
        quads[i-1].dest = to_string(num);
    }
}

// 合并
unordered_set<int> merge(const unordered_set<int>& chain1, const unordered_set<int>& chain2) {
    unordered_set<int> pchain;
    pchain.insert(chain1.begin(), chain1.end());
    pchain.insert(chain2.begin(), chain2.end());
    return pchain;
}

/* 
* 语法制导翻译核心 
* 输入：产生式序号
*/
void syntaxDirectedTranslation(int no) {
    
    Attribute res;	// 归约后产生式左部的属性
	vector<Attribute> attrCache;	// 缓存一下弹出的属性
    Production production = grammar.prods[no];
    SymbolTable* curSymTable = nullptr; // 当前符号表
    unordered_set<int> tmpSet;  // 指令链缓存

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
		backpatch(attrCache[0].NC, quadNo, quads);
        quads.push_back(Quad("End", "_", "_", "_"));
        quadNo++;
        break;

    // Block : { Decls Stmts }
    case 1:
		res.NC = attrCache[1].NC;
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
			// 赋初值
            setIdVal(attrCache[1].lex, 0);
		} else {
            // 重定义
			string errMsg = makeErrMsg(res.token);
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

    // Stmts : Stmts M Stmt
    case 9: 
        backpatch(attrCache[2].NC, attrCache[1].nextInstr, quads);
        res.NC = attrCache[0].NC;
        break; 

    // Stmts : ε
    case 10: break; 

    // Stmt : Var = Bool ;
    case 11: {
        // 取消跳转指令
        quadNo -= 2;
        quads.pop_back();
        quads.pop_back();
        string varName = attrCache[3].lex;
        int tScope = findIdScope(varName);
		if ( tScope != -1 ) {
			attrCache[3].type = blocks[tScope].table[varName].type;
            // 类型不一致，需要转换
            if (attrCache[1].type != attrCache[3].type) {
				string newVar = "t" + to_string(tmpIdx++);
                if(attrCache[3].type == Identifier::INT
                && attrCache[1].type == Identifier::REAL) {
                    quads.push_back(Quad("rtoi", attrCache[1].tempIdName, "_", newVar));
                }
                if(attrCache[1].type == Identifier::INT
                && attrCache[3].type == Identifier::REAL) {
                    quads.push_back(Quad("itor", attrCache[1].tempIdName, "_", newVar));
                }
                if(attrCache[1].type == Identifier::INT
                && attrCache[3].type == Identifier::BOOL) {
                    quads.push_back(Quad("itob", attrCache[1].tempIdName, "_", newVar));
                }
                if(attrCache[3].type == Identifier::INT
                && attrCache[1].type == Identifier::BOOL) {
                    quads.push_back(Quad("btoi", attrCache[1].tempIdName, "_", newVar));
                }
                if(attrCache[1].type == Identifier::BOOL
                && attrCache[3].type == Identifier::REAL) {
                    quads.push_back(Quad("btor", attrCache[1].tempIdName, "_", newVar));
                }
                if(attrCache[3].type == Identifier::BOOL
                && attrCache[1].type == Identifier::REAL) {
                    quads.push_back(Quad("rtob", attrCache[1].tempIdName, "_", newVar));
                }
				quadNo++;
				attrCache[1].tempIdName = newVar;
			}
            res.FC = attrCache[1].FC;
            res.TC = attrCache[1].TC;
            res.NC = { };
			quads.push_back(Quad("=", attrCache[1].tempIdName, "_", "VAR(" + attrCache[3].lex + ")"));
			quadNo++;
			setIdVal(attrCache[3].lex, attrCache[1].value);
		} else {
			string errMsg = makeErrMsg(res.token);
            printSymbolTable(blocks[curScope-1]);
            throw runtime_error(errMsg + "Undefined variant: " + attrCache[3].lex + " !");
		}
    } 
        break; 

    // Stmt : if ( Bool ) M Stmt
    case 12: 
		backpatch(attrCache[3].TC, attrCache[1].nextInstr, quads);
        res.NC = merge(attrCache[3].FC, attrCache[0].NC);
        break; 
        
    // Stmt : if ( Bool ) M Stmt HN else M Stmt
    case 13: 
		backpatch(attrCache[7].TC, attrCache[5].nextInstr, quads);
		backpatch(attrCache[7].FC, attrCache[1].nextInstr, quads);
        tmpSet = merge(attrCache[4].NC, attrCache[3].NC);
        res.NC = merge(tmpSet, attrCache[0].NC);
        break; 
        
    // Stmt : while M ( Bool ) M Stmt
    case 14: 
		backpatch(attrCache[0].NC, attrCache[5].nextInstr, quads);
		backpatch(attrCache[3].TC, attrCache[1].nextInstr, quads);
        res.NC = attrCache[3].FC;
        quads.push_back(Quad("jump", "_", "_", to_string(attrCache[5].nextInstr)));
		quadNo++;
        break; 
        
    // TODO Stmt : break ;
    case 15: 

        break; 
        
    // Stmt : Block
    case 16: 
        res.NC = attrCache[0].NC;
        break; 

    // TODO Var : Var [ int_num ]
    case 17: 

        break; 

    // Var : id
    case 18: 
        res.type = attrCache[0].type;
        res.tempIdName = attrCache[0].tempIdName;
        res.value = attrCache[0].value;
        res.lex = attrCache[0].lex;
        break; 

    // Bool : Bool || M Join
    case 19: 
        res.tempIdName = "t" + to_string(tmpIdx++);
		res.value = (attrCache[3].value || attrCache[0].value);
        backpatch(attrCache[3].FC, attrCache[1].nextInstr, quads);
        res.TC = merge(attrCache[3].TC, attrCache[0].TC); 
        res.FC = attrCache[0].FC;
        break; 

    // Bool : Join
    case 20: 
        res.tempIdName = attrCache[0].tempIdName;
		res.value = attrCache[0].value;
        res.type = attrCache[0].type;
		res.TC = attrCache[0].TC;
		res.FC = attrCache[0].FC;
        break; 

    // Join : Join && M Equality 
    case 21: 
        res.tempIdName = "t" + to_string(tmpIdx++);
		res.value = (attrCache[3].value && attrCache[0].value);
        backpatch(attrCache[3].TC, attrCache[1].nextInstr, quads); 
        res.TC = attrCache[0].TC;
        res.FC = merge(attrCache[3].FC, attrCache[0].FC);  
        break; 

    // Join : Equality
    case 22: 
        res.tempIdName = attrCache[0].tempIdName;
        res.value = attrCache[0].value;
        res.type = attrCache[0].type;
        res.TC = attrCache[0].TC;
        res.FC = attrCache[0].FC;
        break; 

    // Equality : Equality == Rel 
    case 23: 
		res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("==", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = (attrCache[2].value == attrCache[0].value);
        break; 

    // Equality : Equality != Rel 
    case 24: 
		res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("!=", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = (attrCache[2].value != attrCache[0].value);
        break; 

    // Equality : Rel
    case 25: 
        res.tempIdName = attrCache[0].tempIdName;
        res.value = attrCache[0].value;
        res.type = attrCache[0].type;
        res.TC = { quadNo };
		res.FC = { quadNo + 1 };
		quads.push_back(Quad("jTrue", attrCache[0].tempIdName, "_", "_"));
		quadNo++;
		quads.push_back(Quad("jump", "_", "_", "_"));
		quadNo++;
        break; 

    // Rel : Expr < Expr 
    case 26: 
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("<", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = (attrCache[2].value < attrCache[0].value);
        break; 

    // Rel : Expr <= Expr 
    case 27: 
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("<=", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = (attrCache[2].value <= attrCache[0].value);
        break;  

    // Rel : Expr >= Expr 
    case 28: 
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad(">=", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = (attrCache[2].value >= attrCache[0].value);
        break; 

    // Rel : Expr > Expr 
    case 29: 
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad(">", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = (attrCache[2].value > attrCache[0].value);
        break; 

    // Rel : Expr
    case 30: 
        res.type = attrCache[0].type;
        res.tempIdName = attrCache[0].tempIdName;
        res.value = attrCache[0].value;
        break; 

    // Expr : Expr + Term 
    case 31: 
        // bool 不参与加运算
        if(attrCache[0].type == Identifier::BOOL
        || attrCache[2].type == Identifier::BOOL) {
            string errMsg = makeErrMsg(res.token);
            throw runtime_error(errMsg + "Bool var can't be Arithmetic Operated!");
        }
        // 类型不一致，需要转换
        if (attrCache[2].type != attrCache[0].type) {
            string newVar = "t" + to_string(tmpIdx++);
            if(attrCache[2].type == Identifier::INT
            && attrCache[0].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[2].tempIdName, "_", newVar));
            }
            if(attrCache[0].type == Identifier::INT
            && attrCache[2].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[0].tempIdName, "_", newVar));
            }
            res.type = Identifier::REAL;
            quadNo++;
        } else {
            res.type = attrCache[0].type;
        }
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("+", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = attrCache[2].value + attrCache[0].value;
        break; 

    // Expr : Expr - Term 
    case 32: 
        // bool 不参与减运算
        if(attrCache[0].type == Identifier::BOOL
        || attrCache[2].type == Identifier::BOOL) {
            string errMsg = makeErrMsg(res.token);
            throw runtime_error(errMsg + "Bool var can't be Arithmetic Operated!");
        }
        // 类型不一致，需要转换
        if (attrCache[2].type != attrCache[0].type) {
            string newVar = "t" + to_string(tmpIdx++);
            if(attrCache[2].type == Identifier::INT
            && attrCache[0].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[2].tempIdName, "_", newVar));
            }
            if(attrCache[0].type == Identifier::INT
            && attrCache[2].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[0].tempIdName, "_", newVar));
            }
            res.type = Identifier::REAL;
            quadNo++;
        } else {
            res.type = attrCache[0].type;
        }
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("-", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = attrCache[2].value - attrCache[0].value;
        break; 

    // Expr : Term
    case 33: 
        res.type = attrCache[0].type;
        res.tempIdName = attrCache[0].tempIdName;
        res.value = attrCache[0].value;
        res.lex = attrCache[0].lex;
        break; 

    // Term : Term * Unary 
    case 34: 
        // bool 不参与乘运算
        if(attrCache[0].type == Identifier::BOOL
        || attrCache[2].type == Identifier::BOOL) {
            string errMsg = makeErrMsg(res.token);
            throw runtime_error(errMsg + "Bool var can't be Arithmetic Operated!");
        }
        // 类型不一致，需要转换
        if (attrCache[2].type != attrCache[0].type) {
            string newVar = "t" + to_string(tmpIdx++);
            if(attrCache[2].type == Identifier::INT
            && attrCache[0].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[2].tempIdName, "_", newVar));
            }
            if(attrCache[0].type == Identifier::INT
            && attrCache[2].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[0].tempIdName, "_", newVar));
            }
            res.type = Identifier::REAL;
            quadNo++;
        } else {
            res.type = attrCache[0].type;
        }
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("*", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		res.value = attrCache[2].value * attrCache[0].value;
        break; 

    // Term : Term / Unary
    case 35: 
        // bool 不参与除运算
        if(attrCache[0].type == Identifier::BOOL
        || attrCache[2].type == Identifier::BOOL) {
            string errMsg = makeErrMsg(res.token);
            throw runtime_error(errMsg + "Bool var can't be Arithmetic Operated!");
        }
        // 类型不一致，需要转换
        if (attrCache[2].type != attrCache[0].type) {
            string newVar = "t" + to_string(tmpIdx++);
            if(attrCache[2].type == Identifier::INT
            && attrCache[0].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[2].tempIdName, "_", newVar));
            }
            if(attrCache[0].type == Identifier::INT
            && attrCache[2].type == Identifier::REAL) {
                quads.push_back(Quad("itor", attrCache[0].tempIdName, "_", newVar));
            }
            res.type = Identifier::REAL;
            quadNo++;
        } else {
            res.type = attrCache[0].type;
        }
        res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("/", attrCache[2].tempIdName, attrCache[0].tempIdName, res.tempIdName));
		quadNo++;
		// 判断除0错误
        if( isZero(attrCache[0].value) ) {
            string errMsg = makeErrMsg(res.token);
			throw runtime_error(errMsg + "Err: Divided by zero!");
        } else {
            res.value = attrCache[2].value / attrCache[0].value;
        }
        break; 

    // Term : Unary
    case 36: 
        res.type = attrCache[0].type;
        res.tempIdName = attrCache[0].tempIdName;
        res.value = attrCache[0].value;
        break; 

    // Unary : ! Unary 
    case 37: 
		res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("!", attrCache[0].tempIdName, "_", res.tempIdName));
		quadNo++;
		res.value = ( isZero(attrCache[0].value) ) ? 1 : 0;
        break; 

    // Unary : - Unary 
    case 38: 
        res.type = attrCache[0].type;
		res.tempIdName = "t" + to_string(tmpIdx++);
		quads.push_back(Quad("minus", attrCache[0].tempIdName, "_", res.tempIdName));
		quadNo++;
		res.value = -attrCache[0].value;
        break; 

    // Unary : Factor
    case 39: 
        res.type = attrCache[0].type;
        res.tempIdName = attrCache[0].tempIdName;
        res.value = attrCache[0].value;
        break; 

    // Factor : ( Bool ) 
    case 40: 
		// backpatch(attrCache[1].TC, quadNo, quads);
		// backpatch(attrCache[1].FC, quadNo, quads);
        res.type = attrCache[1].type;
        res.tempIdName = attrCache[1].tempIdName;
        res.value = attrCache[1].value;
        break; 

    // Factor : Var 
    case 41: 
		if (findIdScope(attrCache[0].lex) == -1) {
			string errMsg = makeErrMsg(res.token);
            throw runtime_error(errMsg + "Undefined variant " + attrCache[0].lex + " !");
		} else {
            string varName = attrCache[0].lex;
            int scope = findIdScope(varName);
            if( scope != -1) {
                Identifier id = blocks[scope].table[varName];
                res.tempIdName = "VAR(" + attrCache[0].lex + ")";
                res.type = id.type;
                res.value = id.value;
            } else {
                string errMsg = makeErrMsg(res.token);
                printSymbolTable(blocks[curScope-1]);
                throw runtime_error(errMsg + "Undefined variant: " + varName + " !");
            } 
		}
        break; 

    // Factor : int_num   
    case 42:
        res.tempIdName = to_string((int)attrCache[0].value);
        res.value = attrCache[0].value;
        res.type = Identifier::INT;
        break; 

    // Factor : real_num 
    case 43:
        res.tempIdName = to_string(attrCache[0].value);
        res.value = attrCache[0].value;
        res.type = Identifier::REAL;
        break; 
    // Factor : true 
    case 44:
        res.tempIdName = "true";
        res.value = 1;
        res.type = Identifier::BOOL;
        break;
    // Factor : false 
    case 45:
        res.tempIdName = "false";
        res.value = 0;
        res.type = Identifier::BOOL;
        break; 
    // HN : ε
    case 46:
        res.NC = { quadNo };
        quads.push_back(Quad("jump","_", "_", "_"));
        quadNo++;
        break; 
    // M : ε
    case 47:
        res.nextInstr = quadNo;
        break; 
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

void printQuads(vector<Quad>& quads) {
    int size = quads.size();
    for(int i = 0; i < size; ++i) {
        printf("%d ( %s, %s, %s, %s )\n",i+1,
                                    quads[i].opt.c_str(),
                                    quads[i].lhs.c_str(),
                                    quads[i].rhs.c_str(),
                                    quads[i].dest.c_str());
    }
}