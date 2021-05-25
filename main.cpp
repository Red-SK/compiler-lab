#include "tokenizer.h"
#include "parser.h"
#include <iostream>
#include <vector>
#include <cstdio>

using namespace std;

int main(int argc, char** argv) {
    string contentPath;
    if(argc == 1) {
        contentPath = readFile("test/test.lex"); 
    } else {
        contentPath = readFile(argv[1]); 
    }
    test4Tokenizer(contentPath);
    test4Parser();
    return 0;
}