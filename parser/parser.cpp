#include "parser.h"
#include "tokenizer.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "terminator.list"
#include "non_terminator.list"

CanonicalCollection CC; // LR1项目集规范族
Grammar grammar; // 文法

// FIRST集 [符号]->[终结符集]
map<int,set<int>> firstSet;
// NULLABLE集（可能会推导出ε的非终结符）
set<int> nullableSet;

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
        grammar.T.push_back(i);
    }
    // 载入非终结符（需要偏移）
    for(int i = Parser::Program;i<=Parser::Factor;i++) {
        grammar.N.push_back(i-100);
    }
    // 载入产生式
    loadProduction("./parser/production.list");
    // 求NULLABLE集和FIRST集
    getNullableSet();
    getFirstSet();
    // 构建DFA
    DFA();
    // 构建预测分析表
    buildPredictTable();
}

// 判断产生式右部是否全是非终结符
static bool isProdRightAllNonT(Production& production) {
    for(int symbol : production.right) {
        if(isTerminator(symbol)) {
            return false;
        }
    }
    return true;
}

// 判断产生式右部是否全属于nullable
static bool isProdRightAllBelong2Nullable(Production& production) {
    for(int symbol : production.right) {
        // 没找到，不属于
        if(nullableSet.find(symbol) == nullableSet.end()) {
            return false;
        }
    }
    return true;
}

// 求NULLABLE集
void getNullableSet() {
    set<int> last;
    do {
        last = nullableSet;
        for(Production production :grammar.prods) {
            if(production.right.size() && production.right[0] == Parser::epsilon) {
                nullableSet.insert(production.left);
            }
            else if(isProdRightAllNonT(production) && isProdRightAllBelong2Nullable(production)) {
                nullableSet.insert(production.left);
            }
        }
    } while(last != nullableSet);
}

// 初始化FIRST集
static void initFirstSet() {
    set<int> empty; empty.clear();
    for(int i : grammar.N) {
        firstSet[i] = empty;
    }
    for(int i : grammar.T) {
        firstSet[i] = empty;
        firstSet[i].insert(i);
    }
}

void getFirstSet() {
    initFirstSet();
    map<int,set<int>> last;
    do {
        last = firstSet;
        for(Production production : grammar.prods) {
            int N = production.left;
            for(int symbol : production.right) {
                // 首个符号为终结符，直接添加
                if(isTerminator(symbol)) {
                    firstSet[N].insert(symbol);
                    break;
                }
                // 非终结符
                if(!isTerminator(symbol)) {
                    // 求firstSet[N]和firstSet[symbol]的并集
                    // 放入firstSet[N]
                    set_union(firstSet[N].begin(), firstSet[N].end(), firstSet[symbol].begin(), firstSet[symbol].end(), inserter(firstSet[N], firstSet[N].begin()));
                    if(nullableSet.find(symbol) == nullableSet.end()) break;
                }
            }
        }
    } while(last != firstSet);
}

// 获取产生式左部为symbol的项目集
// symbol: 上一个式子·后的非终结符
// first: symbol后的符号求出来的first集
static vector<LR1Item>* getItemsByN(int symbol, set<int>& first) {
    vector<LR1Item> *items = new vector<LR1Item>;
    LR1Item item;
    for(auto& prod : grammar.prods) {
        // 找到对应产生式
        if(prod.left == symbol) {
            for(int i : first) {
                item.location = 0;
                item.next = i;
                item.production = prod;
                items->push_back(item);
            }       
        }
    }
    return items;
}

// 获取 · 后的非终结符之后的first集
static set<int>* getFirstSetAfterSymbol(LR1Item& item) {
    set<int> *first = new set<int>;
    // 模拟吃了一个符号
    int loc = item.location + 1;
    int size = item.production.right.size();
    while(loc < size) {
        int symbol = item.production.right[loc];
        // 终结符 直接是first
        if(isTerminator(symbol)) {
            first->insert(symbol);
            return first;
        } else {
            set<int> s = firstSet[symbol];
            for(int i : s) {
                first->insert(i);
            }
            // 该非终结符不在nullable内
            if(nullableSet.find(symbol) == nullableSet.end()) return first;                
            loc++;
        }
    }
    first->insert(item.next);
    return first;
}

static bool isLR1ItemSetEqual(LR1ItemSet& s1, LR1ItemSet& s2) {
    if(s1.items.size() != s2.items.size()) return false;
    int len = s1.items.size();
    for(int i = 0; i < len; i++) {
        bool f = false;
        for(int j = 0; j < len; j++) {
            if(s1.items[i] == s2.items[j]) {
                f = true;
                break;
            }
        }
        if(!f) return false;
    }
    return true;
}

static void printLR1ItemSet(LR1ItemSet& s) {
    cout << "-----------------------" << endl;
    for(auto& item : s.items) {
        cout << nonTerminatorList[item.production.left-100] << " -> ";
        int rsize = item.production.right.size();
        for(int i = 0; i < rsize; i++) {
            if(item.location == i) cout << "· ";
            if(isTerminator(item.production.right[i]))
                cout << terminatorList[item.production.right[i]] << " ";
            else
                cout << nonTerminatorList[item.production.right[i]-100] << " ";
        }
        cout << ", " << terminatorList[item.next] << endl;
    }
    cout << "-----------------------" << endl;
}

// FIXME:求项目集闭包
void closure(LR1ItemSet& s) {
    LR1ItemSet last;
    int cnt = 0;
    do {
        last = s;
        //printLR1ItemSet(last);
        for(int i = cnt; i < s.items.size(); i++) {
            LR1Item item = s.items[i];
            // 移进 or 待约
            if(item.location < item.production.right.size()) {
                // 取出在 · 后的非终结符
                int symbol = item.production.right[item.location];
                // 如果是非终结符，求闭包
                if(!isTerminator(symbol)) {
                    set<int> *first = getFirstSetAfterSymbol(item);
                    vector<LR1Item> *newItems = getItemsByN(symbol,*first);
                    // 插入新的项目
                    for(auto& it : *newItems) {
                        s.items.push_back(it);
                    }
                }
            }
        }
        cnt++;
        //printLR1ItemSet(s);
    } while(!isLR1ItemSetEqual(last,s));
}

// TODO: 构建DFA
void DFA() {
    // 构建初始项目集
    LR1Item startItem;
    startItem.production = grammar.prods[0];
    startItem.location = 0;
    startItem.next = Parser::eof;
    LR1ItemSet I0;
    I0.items.push_back(startItem);
    closure(I0);
    // 加入初始有效项目集
    CC.itemSets.push_back(I0);
}

// TODO: 构建预测分析表
void buildPredictTable() {

}

// 测试输出用
void test4Parser() {
    printf("-------------------------\n");
    printf("|         Parser         |\n");
    printf("-------------------------\n");
    initGrammar();
    int num = grammar.prods.size();
    for(int i = 0;i<num;i++) {
        cout << i << ": " << nonTerminatorList[grammar.prods[i].left-100] << " ->";
        int len = grammar.prods[i].right.size();
        for(int j=0;j<len; j++) {
            int right = grammar.prods[i].right[j];
            if(isTerminator(right)) 
                cout << " " << terminatorList[right];
            else
                cout << " " << nonTerminatorList[right-100];
        }
        cout << endl;
    }
    cout << "Nullable: { ";
    for(auto i : nullableSet) {
        cout  << nonTerminatorList[i-100] << " ";
    }
    cout << "}" << endl;
    cout << "First Set: " << endl;
    for(auto& N : firstSet) {
        if(isTerminator(N.first)) {
            printf("FIRST(%s) = { ",terminatorList[N.first].c_str());
        } else {
            printf("FIRST(%s) = { ",nonTerminatorList[N.first-100].c_str());
        }
        for(auto symbol : N.second) {
            if(isTerminator(symbol)) {
                cout << terminatorList[symbol] << " ";
            } else {
                cout << nonTerminatorList[symbol-100] << " ";
            }
        }
        printf("}\n");
    }
    LR1ItemSet I0 = CC.itemSets[0];
    printLR1ItemSet(I0);
}