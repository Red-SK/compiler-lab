#include "parser.h"
#include "tokenizer.h"
#include "terminator.list"
#include "non_terminator.list"
#include <cstdio>
#include <fstream>
#include <iostream>

CanonicalCollection CC; // LR1项目集规范族
Grammar grammar; // 文法
FIRST firstSet;  // FIRST集
FOLLOW followSet; //FOLLOW集

/* DFA队列， 用于存储待转移的有效项目集 */
queue<pair<LR1ItemSet,int>> shiftQueue;

/* action表和goto表 */
pair<int,int> ACTION[100][100]; // first是分析动作，second是转移状态或者产生式序号
int GOTO[100][100]; 

/* 分析栈 */
stack<pair<int,int>> aStack ; // first是状态，second是符号

// 找到终结符字符串对应的枚举值
static int findTerminator(string T) {
    for(int i = Parser::epsilon;i<=Parser::eof;i++) {
        if(terminatorList[i] == T) {
            return i;
        }
    }  
    return -1;
}

// 找到非终结符字符串对应的枚举值
static int findNonTerminator(string N) {
    for(int i = Parser::Program;i<=Parser::Factor;i++) {
        if(nonTerminatorList[i-100] == N) {
            return i;
        }
    }
    return -1;
}

// 判断是否为终结符
static bool isTerminator(int symbol) {
    if(symbol >= 0 && symbol < 100) {
        return true;
    }
    return false;
}

// 生成产生式
static Production* makeProduction(vector<string>& symbol) {
    Production* production = new Production;
    int left = findNonTerminator(symbol[0]);
    if(left == -1) {
        cout << "No such NonTerminator: " << symbol[0] << "!" << endl;
    }
    production->left = left;
    int len = symbol.size();
    // [0]是左部 [1]是":"
    for(int i = 2; i < len; i++) {
        // 过滤空格
        if(symbol[i][0] != ' '  && symbol[i][0] != '\r' 
        && symbol[i][0] != '\n' && symbol[i] != "") {
            // 去除回车换行
            if(i == len - 1) {
                int cnt = 0;
                int strLen = symbol[i].size();
                for(int j = strLen-1; j >= 0 ; j--) {
                    if(symbol[i][j] == '\n' || symbol[i][j] == '\r') {
                        cnt++;
                    }
                }
                symbol[i] = symbol[i].substr(0,strLen-cnt);
            }
            if(findNonTerminator(symbol[i]) != -1) {
                production->right.push_back(findNonTerminator(symbol[i]));
            } else if(findTerminator(symbol[i]) != -1) {
                production->right.push_back(findTerminator(symbol[i]));
            }     
        }
    }
    return production;
}

// 读取产生式配置文件
static void loadProduction(const char* path) {
    ifstream in(path);
    string line;
 
    if(in) {
        while(getline(in,line)) { 
            vector<string> symbol = split(line," ");
            grammar.prods.push_back(*makeProduction(symbol));
        }
    } else {
        printf("No such file: %s!\n",path);
    }
}

// 初始化文法
static void initGrammar() {
    printf("Start loading the grammar...\n");
    // 载入终结符
    for(int i = Parser::epsilon;i<=Parser::eof;i++) {
        grammar.T.push_back(terminatorList[i]);
    }
    // 载入非终结符（需要偏移）
    for(int i = Parser::Program;i<=Parser::Factor;i++) {
        grammar.N.push_back(nonTerminatorList[i-100]);
    }
    // 载入产生式
    loadProduction("./parser/production.list");
}

// 测试输出用
void test4parser() {
    initGrammar();
    int num = grammar.prods.size();
    for(int i = 0;i<num;i++) {
        cout << nonTerminatorList[grammar.prods[i].left-100] << " ->";
        int len = grammar.prods[i].right.size();
        for(int j=0;j<len; j++) {
            int right = grammar.prods[i].right[j];
            if(isTerminator(right)) 
                cout  << " " << terminatorList[right];
            else
                cout  << " " << nonTerminatorList[right-100];
        }
        cout << endl;
    }
}