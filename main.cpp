#include "tokenizer.h"
#include "parser.h"
#include <iostream>
#include <vector>
#include <cstdio>

using namespace std;

vector<Token> tokens;
#include "token.list"

void testTokenizer();

int main(int argc, char** argv) {
    string contentPath;
    if(argc == 1) {
        contentPath = readFile("test/test.lex"); 
    } else {
        contentPath = readFile(argv[1]); 
    }
    lexToTokens(contentPath, tokens);
    testTokenizer();
    test4parser();
    return 0;
}

void testTokenizer() {
    printf("-------------------------\n");
    printf("|       Tokenizer       |\n");
    printf("-------------------------\n");
    for(Token token : tokens) {
        printf("<@%d:%d: '%s', Line %d, %d:%d, %s>\n",
                token.start,token.end,token.lex.c_str(),token.curLine,
                token.curStart,token.curEnd,tokenList[token.type].c_str());
    }
}