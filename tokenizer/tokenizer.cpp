#include "tokenizer.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

char* readFile(const char* path) {
    int total = 0;
    ifstream srcFile(path,ios::in); 
    if(!srcFile) {
        cout << "error opening source file." << endl;
        return NULL;
    }
    cout << "Start loading the file..." << endl;
    struct stat fileStat;
    stat(path, &fileStat);
    size_t fileSize = fileStat.st_size;
    cout << path << ": " << fileSize << " Bytes." << endl;
    char* fileContent = (char*) malloc(fileSize + 1);
    if(fileContent == NULL){
        cout << "Couldn't allocate memory for reading file " << path << "." << endl;
    }
    char c;   
    while(srcFile >> c) {
        fileContent[total++] = c;
    }     
    fileContent[total] = '\0';
    srcFile.close();
    return fileContent;
}