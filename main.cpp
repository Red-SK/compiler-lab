#include "tokenizer.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    cout << "Welcome!" << endl;
    char* content = readFile("test.lex");
    int cnt = 0;
    while(content[cnt] != '\0') {
        cout << content[cnt++];
    }
    cout << endl;
    return 0;
}