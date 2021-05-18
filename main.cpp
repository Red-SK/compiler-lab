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
    char* content = readFile("test.lex"); 
    int cnt = lexToTokens(content, tokens);
    testTokenizer();
    cout << cnt << endl;
    return 0;
}