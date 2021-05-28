#include "parser.h"
#include "tokenizer.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "terminator.list"
#include "non_terminator.list"

// 导入外部的Tokens
extern vector<Token> tokens;

CanonicalCollection CC; // LR1项目集规范族
Grammar grammar; // 文法

// FIRST集 [符号]->[终结符集]
map<int,set<int>> firstSet;
// NULLABLE集（可能会推导出ε的非终结符）
set<int> nullableSet;

// DFA队列,用于存储待转移的有效项目集
queue<pair<LR1ItemSet,int>> shiftQueue;

// action表和goto表
// 行是项目集序号
// 列是终结符 or 非终结符编号
pair<int,int> ACTION[1000][100]; // first是分析动作，second是转移状态或者产生式序号
int GOTO[1000][100]; 

// 分析栈
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
        grammar.N.push_back(i);
    }
    // 载入产生式
    loadProduction("./parser/production.list");
    // 求NULLABLE集和FIRST集
    getNullableSet();
    getFirstSet();
    cout << "Finish computing NullableSet & FirstSet!" << endl;
    // 构建DFA
    DFA();
    cout << "Finish building DFA!" << endl;
    // 构建预测分析表
    buildPredictTable();
    cout << "Finish building PredictTable!" << endl;
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
                // 插入不为空的
                if(i != Parser::epsilon) first->insert(i);
            }
            // 该非终结符不在nullable内
            if(nullableSet.find(symbol) == nullableSet.end()) {
                return first;
            }
            loc++;
        }
    }
    first->insert(item.next);
    return first;
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

// 判断两个项目集是否相同（用来判断项目集是否变化）
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

// 打印项目
static void printLR1Item(LR1Item& item) {
    cout << nonTerminatorList[item.production.left-100] << " -> ";
    int rsize = item.production.right.size();
    for(int i = 0; i < rsize; i++) {
        if(item.location == i) cout << "· ";
        if(isTerminator(item.production.right[i]) &&
           item.production.right[i] != Parser::epsilon)
            cout << terminatorList[item.production.right[i]] << " ";
        else
            cout << nonTerminatorList[item.production.right[i]-100] << " ";
    }
    if(item.location == item.production.right.size()) {
        cout << "· ";
    }
    cout << ", " << terminatorList[item.next] << endl;
}

// 打印项目集
static void printLR1ItemSet(vector<LR1Item>& items) {
    cout << "-----------------------" << endl;
    for(auto& item : items) {
        printLR1Item(item);
    }
    cout << "-----------------------" << endl;
}

// 判断项目是否在项目集内
static bool isItemInItemSets(LR1Item& item, vector<LR1Item>& items) {
    for(auto& it : items) {
        if(it == item) return true;
    }
    return false;
}

// 求项目集闭包
void closure(LR1ItemSet& s) {
    LR1ItemSet last;
    int cnt = 0;
    do {
        last = s;
        int size = s.items.size();
        for(int i = cnt; i < size; i++) {
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
                        // 确保没有才插入
                        if(!isItemInItemSets(it,s.items)) {
                            // cout << "insert: ";
                            // printLR1Item(it);
                            // printLR1ItemSet(s.items);
                            s.items.push_back(it);
                        }                         
                    }
                }
            }
        }
        cnt++;
        //printLR1ItemSet(s);
    } while(!isLR1ItemSetEqual(last,s));
}

// 吃入一个字符，到达一个新状态
void go(LR1ItemSet& src, int symbol, LR1ItemSet& dst) {
    // 找到 · 后字符为symbol的项目
    for(auto& item : src.items) {
        if(item.location < item.production.right.size() &&
           item.production.right[item.location] == symbol &&
           item.production.right[0] != Parser::epsilon) {
            LR1Item newItem = item;
            newItem.location++;
            dst.items.push_back(newItem);
            //printLR1ItemSet(dst.items);
        }
    }
    // 求新的项目集的闭包
    closure(dst);
}

// 判断是否在项目集规范族中，若在返回序号
int isInCanonicalCollection(LR1ItemSet& is) {
    for (int i = 0; i < CC.itemSets.size(); i++) {
        if(isLR1ItemSetEqual(is, CC.itemSets[i])) {
            return i;
        }
    }
    // 不存在
    return -1;
}

// 构建DFA和项目集规范族
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
    // 把新加入的有效项目集加入待扩展队列中
    shiftQueue.push(pair<LR1ItemSet, int>(I0, 0));
    while(!shiftQueue.empty()) {
        // 取出队首元素
        LR1ItemSet& src = shiftQueue.front().first;
        int srcIndex = shiftQueue.front().second;
        // 遍历每个终结符
        for(int i = 0; i < grammar.T.size(); i++) {
            LR1ItemSet nextSet;
            go(src,grammar.T[i],nextSet);
            if(nextSet.items.size() > 0) {
                int pos = isInCanonicalCollection(nextSet);
                // 如果有新的项目集
                if(pos == -1) {
                    int index = CC.itemSets.size();
                    CC.itemSets.push_back(nextSet);
                    // 把新加入的有效项目集加入待扩展队列中
                    shiftQueue.push(pair<LR1ItemSet, int>(nextSet, index));
                    // srcIndex，吃了grammar.T[i]，到达index
                    CC.g[srcIndex].push_back(pair<int, int>(grammar.T[i], index));
                } else { // 原有的项目集
                    CC.g[srcIndex].push_back(pair<int, int>(grammar.T[i], pos));
                }            
            }
        }
        // 遍历每个非终结符
        for(int i = 0; i < grammar.N.size(); i++) {
            LR1ItemSet nextSet;
            go(src,grammar.N[i],nextSet); 
            if(nextSet.items.size() > 0) {
                int pos = isInCanonicalCollection(nextSet);
                // 如果有新的项目集
                if(pos == -1) {
                    int index = CC.itemSets.size();
                    CC.itemSets.push_back(nextSet);
                    // 把新加入的有效项目集加入待扩展队列中
                    shiftQueue.push(pair<LR1ItemSet, int>(nextSet, index));
                    // srcIndex，吃了grammar.T[i]，到达index
                    CC.g[srcIndex].push_back(pair<int, int>(grammar.N[i], index));
                } else { // 原有的项目集
                    CC.g[srcIndex].push_back(pair<int, int>(grammar.N[i], pos));
                }            
            }         
        }
        shiftQueue.pop();
    }
}

// 初始化ACTION表和GOTO表
static void initPredictTable() {
    int cSize = CC.itemSets.size();
    for(int i = 0; i < cSize; i++) {
        for(int j = 0; j <= Parser::eof; j++) {
            ACTION[i][j] = make_pair(Parser::Error,-1);
        }
        for(int j = Parser::Program; j <= Parser::Factor; j++) {
            GOTO[i][j-100] = -1;
        }
    }
}

// 构建预测分析表
void buildPredictTable() {
    initPredictTable();
    int cSize = CC.itemSets.size();
    // 遍历每一个项目集
    for(int i = 0; i < cSize; i++) {
        LR1ItemSet& items = CC.itemSets[i];
        // 构建ACTION表
        for(auto& item : items.items) {
            int symbol = item.production.right[item.location];
            // 移进 or 待约项
            // 形如 A -> ε 的，直接规约就行
            if(item.location < item.production.right.size()
            && item.production.right[0] != Parser::epsilon) {
                // ACTION表只有终结符
                if(isTerminator(symbol)) {
                    for(int j = 0; j < CC.g[i].size(); j++) {
                        pair<int, int> p = CC.g[i][j];
                        if (p.first == symbol) {
                            // 在吃入的symbol位置填入S_p.second
                            // 代表吃入symbol，跳到I_p.second
                            ACTION[i][symbol].first = Parser::Shift;
                            ACTION[i][symbol].second = p.second;
                            break;
                        }
                    }
                }
            }
            // 规约项
            else {
                // Accept项
                if(item.production.left == grammar.prods[0].left
                   && item.next == Parser::eof) {
                    ACTION[i][item.next] = make_pair(Parser::Accept, 0);
                } else {                   
                    for(int j = 0; j < grammar.prods.size(); j++) {
                        if(item.production == grammar.prods[j]) {
                            ACTION[i][item.next].first = Parser::Reduce;
                            // 在吃入的展望符位置填入Rj
                            // 代表第j个产生式的规约动作
                            ACTION[i][item.next].second = j;
                            break;
                        }
                    }                                       
                }
            }
        }
        // 构建GOTO表
        for (int k = 0; k < CC.g[i].size(); k++) {
            pair<int, int> p = CC.g[i][k];
            int symbol = p.first;
            // GOTO表都是非终结符
            if(!isTerminator(symbol)) {
                // 非终结符从100开始枚举，这里要减掉
                GOTO[i][symbol-100] = p.second;
            }
        }
    }
}

// 打印DFA所有项目集
static void printDFA() {
    for (int i = 0; i < CC.itemSets.size(); i++) {
        printf("LR1ItemSet %d:\n", i);
        printLR1ItemSet(CC.itemSets[i].items);
        for (int j = 0; j < CC.g[i].size(); j++) {
            pair<int, int> p= CC.g[i][j];
            if(isTerminator(p.first)) {
                printf("eat %s to LR1ItemSet %d\n", terminatorList[p.first].c_str(), p.second);
            } else {
                printf("eat %s to LR1ItemSet %d\n", nonTerminatorList[p.first-100].c_str(), p.second);
            }      
        }     
    }
}

// 传入产生式索引，打印产生式
static void printProduction(int i) {
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

// FIXME: 打印预测表
static void printPredictTable() {
    for (int i = 0; i < grammar.T.size() / 2; i++)
        printf("\t");
    printf("ACTION");
    for (int i = 0; i < grammar.N.size() / 2 + grammar.T.size() / 2 + 1; i++)
        printf("\t");
    printf("GOTO\n");
    printf("\t");
    for (int i = 1; i  < grammar.T.size(); i++) {
        printf("%s\t", terminatorList[grammar.T[i]].c_str());
    }
    printf("|\t");
    for (int i = 1; i  < grammar.N.size(); i++) {
        printf("%s\t", nonTerminatorList[grammar.N[i]-100].c_str());
    }
    printf("\n");
    for (int i = 0; i < CC.itemSets.size(); i++) {
        printf("%d\t", i);
        for (int j = 1; j < grammar.T.size(); j++) {
            if (ACTION[i][j].first == Parser::Shift) {
                printf("S%d\t", ACTION[i][j].second);
            } else if (ACTION[i][j].first == Parser::Reduce) {
                printf("R%d\t", ACTION[i][j].second);
            } else if (ACTION[i][j].first == Parser::Accept) {
                printf("ACC\t");
            } else {
                printf("\t");
            }
        }
        printf("|\t");
        for (int j = 1; j < grammar.N.size(); j++) {
            if (GOTO[i][j]) {
                printf("%d\t", GOTO[i][j]);
            } else {
                printf("\t");
            }
            
        }
        printf("\n");
    }
}

// 打印ACTION表的row状态行
static void printACTIONRow(int row) {
    printf("I%d:\n",row);
    int ac;
    for(int i = 0; i < grammar.T.size(); i++) {
        ac = ACTION[row][i].first;
        switch(ac) {
            case Parser::Accept:
                printf("%s\tAcc\t\n",terminatorList[grammar.T[i]].c_str());
                break;
            case Parser::Shift:
                printf("%s\tS%d\t\n",terminatorList[grammar.T[i]].c_str(),ACTION[row][i].second);
                break;
            case Parser::Reduce:
                printf("%s\tR%d\t\n",terminatorList[grammar.T[i]].c_str(),ACTION[row][i].second);
                break;
            case Parser::Error:
                printf("%s\tErr\t\n",terminatorList[grammar.T[i]].c_str());
                break;
        }
    }
}

// 语法分析（使用分析栈）
void syntaxParser() {
    cout << "Start Parsing..." << endl;
    Token end;
    end.type = END_OF_STRING;
    end.curEnd = end.curLine = end.curStart = end.end = end.start = -1;
    end.lex = "$";
    tokens.push_back(end);
    aStack.push(pair<int,int>(0, Parser::eof));
    // 开始分析
    int ip = 0;
    int step = 1;
    while(1) {
        int topState = aStack.top().first;
        Token& curToken = tokens[ip];
        int symbol = curToken.type;
        cout << ACTION[topState][symbol].first << " " << ACTION[topState][symbol].second << endl;
        printLR1ItemSet(CC.itemSets[topState].items);
        printACTIONRow(topState);
        // 移进
        if (ACTION[topState][symbol].first == Parser::Shift) {
            aStack.push(pair<int, int>(ACTION[topState][symbol].second, symbol));
            printf("%d Shift %s\n", step++, terminatorList[symbol].c_str());
            ip++;
        } 
        // 规约
        else if (ACTION[topState][symbol].first == Parser::Reduce) { 
            int rIndex = ACTION[topState][symbol].second;
            Production& P = grammar.prods[rIndex];
            // 弹出产生式(除了A -> ε)
            if(P.right[0] != Parser::epsilon) {
                for (int i = 0; i < P.right.size(); i++) {
                    aStack.pop();
                }
            }
            printf("%d Reduce %d: ", step++, rIndex);
            printProduction(rIndex);
            topState = aStack.top().first;
            int N = P.left;
            aStack.push(pair<int, int>(GOTO[topState][N-100], N));
        } 
        // 接受
        else if (ACTION[topState][symbol].first == Parser::Accept) {
            printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf("Ohhhhhhhhhhhhhh!!!!!!!!!!!!!!\n");
            printf("ACCEPT!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");           
            return;
        } 
        // 错误
        else {
            printf("*****************************\n");  
            printf("Syntax Error!\n");
            printf("Please check '%s' at %d:%d.\n",
                    curToken.lex.c_str(),
                    curToken.curLine, 
                    curToken.curStart);
            printf("The ACTION is %d, STATE is %d\n",
                    ACTION[topState][symbol].first,
                    ACTION[topState][symbol].second);
            printf("*****************************\n"); 
            return;
        }        
    }
}

// 测试输出用
void test4Parser() {
    printf("-------------------------\n");
    printf("|         Parser         |\n");
    printf("-------------------------\n");
    initGrammar();
    int num = grammar.prods.size();
    for(int i = 0;i<num;i++) {
        printProduction(i);
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
    printf("Finish building the DFA!\nCC size: %ld\n", CC.itemSets.size());
    // printPredictTable();
    syntaxParser();
}