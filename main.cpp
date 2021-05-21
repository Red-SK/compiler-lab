#include "tokenizer.h"
#include <iostream>
#include <vector>

using namespace std;

vector<Token> tokens;
#include "token.list"

void testTokenizer() {
    for(auto& token : tokens) {
        cout << "<" << token.lex << "," << tokenList[token.type] << ">" << endl;
    }
}

int main(int argc, char** argv) {
    cout << "Welcome!" << endl;
    char* contentPath;
    if(argc == 1) {
        contentPath = readFile("test/test.lex"); 
    } else {
        contentPath = readFile(argv[1]); 
    }
    int cnt = lexToTokens(contentPath, tokens);
    testTokenizer();
    cout << cnt << endl;
    return 0;
}