#include "parser.h"
#include "tokenizer.h"

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
