#include "tokenizer.h"
#include "parser.h"
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <sys/time.h>

using namespace std;

int main(int argc, char** argv) {
    struct timeval start,end;
    gettimeofday(&start,NULL);
    string contentPath;
    if(argc == 1) {
        contentPath = readFile("test/test.lex"); 
    } else {
        contentPath = readFile(argv[1]); 
    }
    test4Tokenizer(contentPath);
    test4Parser();
    gettimeofday(&end,NULL);
    printf("cost time : %ld ms\n",(end.tv_sec-start.tv_sec)*1000 + (end.tv_usec-start.tv_usec)/1000);
    return 0;
}