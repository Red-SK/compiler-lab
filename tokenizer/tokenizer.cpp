#include "tokenizer.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include "token.list"

int strStart = 0; // 全局lex起始符
int strEnd = 0; // 全局lex终止符
int curLine = 1; // 起始行
int totalCharsExceptCur = 0; // 除了当前行，其他行字符数总和

vector<Token> tokens; // 输出的Token集

// 读取文本文件
string readFile(const char* path) {
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
    cout << path << " (size): " << fileSize << " Bytes." << endl;
	stringstream ss;
	ss << srcFile.rdbuf();
	string fileContent = ss.str();
    srcFile.close();
    return fileContent;
}

// 字符串转Token
Token* makeToken(string str, TokenType type) {
    Token* token = new Token;
    token->lex = str;
    token->type = type;
    token->start = strStart;
    token->end = strEnd;
    token->curLine = curLine;
    token->curStart = strStart - totalCharsExceptCur;
    token->curEnd = strEnd - totalCharsExceptCur;
    return token;
}

// 判断是否为数字字符(0-9)
static bool isDigit(char c) {
    if(c >= '0' && c <= '9') return true;
    return false;
}

// 判断是否为字母字符(a-zA-Z)
static bool isCharacter(char c) {
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return true;
    return false;
}

// lex转tokens
int lexToTokens(string& content, vector<Token>& tokens) {
    while(content[strStart] != '\0') {
        // skip blank space
        if(content[strStart] == ' ' || content[strStart] == '\n' || content[strStart] == '\r') {
            if(content[strStart] == '\n') {
                curLine++;
                totalCharsExceptCur = strEnd + 1;
            }
            strStart = ++strEnd;   
        }
        else if(content[strStart] == '+') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ADD));
            strStart = ++strEnd;
        }
        else if(content[strStart] == '-') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),SUB));
            strStart = ++strEnd;
        }
        else if(content[strStart] == '*') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),MUL));
            strStart = ++strEnd;
        }
        else if(content[strStart] == '/') {
            if(content[strStart+1] == '/') {
                while(content[strStart] != '\n') strStart = ++strEnd;
                curLine++;
                totalCharsExceptCur = strEnd + 1;
                strStart = ++strEnd; 
            }else {
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),DIV));
                strStart = ++strEnd;
            }
        }
        else if(content[strStart] == '(') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),CIR_LEFT_BRACKET));
            strStart = ++strEnd;
        }
        else if(content[strStart] == ')') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),CIR_RIGHT_BRACKET));
            strStart = ++strEnd;
        }
        else if(content[strStart] == '[') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),SQ_LEFT_BRACKET));
            strStart = ++strEnd;
        }
        else if(content[strStart] == ']') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),SQ_RIGHT_BRACKET));
            strStart = ++strEnd;
        }
        else if(content[strStart] == '{') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),LEFT_BLOCK));
            strStart = ++strEnd;
        }
        else if(content[strStart] == '}') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),RIGHT_BLOCK));
            strStart = ++strEnd;
        }
        else if(content[strStart] == '<') {
            if(content[strStart+1] == '=') {
                strEnd++;
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),LESS_EQUAL));              
                strStart = ++strEnd;
            } else {
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),LESS));
                strStart = ++strEnd;
            }          
        }
        else if(content[strStart] == '>') {
            if(content[strStart+1] == '=') {
                strEnd++;
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),GREATER_EQUAL));              
                strStart = ++strEnd;
            } else {
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),GREATER));
                strStart = ++strEnd;
            }          
        }
        else if(content[strStart] == '=') {
            if(content[strStart+1] == '=') {
                strEnd++;
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),EQUAL));                
                strStart = ++strEnd;
            } else {
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ASSIGN));
                strStart = ++strEnd;
            }          
        }
        else if(content[strStart] == '!') {
            if(content[strStart+1] == '=') {
                strEnd++;
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),NOT_EQUAL));              
                strStart = ++strEnd;
            } else {
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),NOT));
                strStart = ++strEnd;
            }      
        }
        else if(content[strStart] == '&') {
            if(content[strStart+1] == '&') {
                strEnd++;
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),AND));                
                strStart = ++strEnd;
            } else {
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),UNKNOWN));
                strStart = ++strEnd;
            }
        }
        else if(content[strStart] == '|') {
            if(content[strStart+1] == '|') {
                strEnd++;
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),OR));                
                strStart = ++strEnd;
            } else {
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),UNKNOWN));
                strStart = ++strEnd;
            }
        }
        else if(content[strStart] == ';') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),SEMICOLON));
            strStart = ++strEnd;
        }
        else if(content[strStart] == ',') {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),COMMA));
            strStart = ++strEnd;
        }
        // 数字
        else if(isDigit(content[strStart])) {
            while(isDigit(content[strEnd])) strEnd++;
            if(content[strEnd] == '.') { // 实数
                strEnd++;
                while(isDigit(content[strEnd])) strEnd++;
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart),REAL_NUM));
            } else { // 整数
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart),INT_NUM));
            }
            strStart = strEnd;
        }
        // 关键字和id
        else if(isCharacter(content[strStart])) {
            /* 关键字 */
            if(content[strStart] == 'b') {
                if(content[strStart+1] == 'o' && content[strStart+2] == 'o' && 
                   content[strStart+3] == 'l' && !isCharacter(content[strStart+4])) {
                    strEnd += 3;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),BOOL));
                }
                else if(content[strStart+1] == 'r' && content[strStart+2] == 'e' &&
                        content[strStart+3] == 'a' && content[strStart+4] == 'k' &&
                        !isCharacter(content[strStart+5])) {
                    strEnd += 4;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),BREAK));
                }
                else {
                    while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;                      
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                }
            }
            else if(content[strStart] == 'i') {
                // int
                if(content[strStart+1] == 'n' && content[strStart+2] == 't' &&
                   !isCharacter(content[strStart+3])) {
                    strEnd += 2;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),INT));
                }
                // if --> if()
                else if(content[strStart+1] == 'f' && !isCharacter(content[strStart+2])) {
                    strEnd += 1;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),IF));
                }
                else {
                    while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;   
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                }
            }
            else if(content[strStart] == 'e') {
                if(content[strStart+1] == 'l' && content[strStart+2] == 's' &&
                   content[strStart+3] == 'e' && !isCharacter(content[strStart+4])) {
                    strEnd += 3;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ELSE));
                }
                else {
                    while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;   
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                }
            }
            else if(content[strStart] == 'f') {
                if(content[strStart+1] == 'a' && content[strStart+2] == 'l' &&
                   content[strStart+3] == 's' && content[strStart+4] == 'e' && 
                   !isCharacter(content[strStart+5])) {
                    strEnd += 4;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),FALSE));
                }
                else {
                    while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;   
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                }
            }
            else if(content[strStart] == 'r') {
                if(content[strStart+1] == 'e') {
                    if(content[strStart+2] == 'a' && content[strStart+3] == 'l' &&
                       !isCharacter(content[strStart+4])) {
                        strEnd += 3;
                        tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),REAL));
                    }
                    else {
                        while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;    
                        tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                    }
                }
                else {
                    while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;    
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                }
            }
            else if(content[strStart] == 't') {
                if(content[strStart+1] == 'r' && content[strStart+2] == 'u' &&
                   content[strStart+3] == 'e' && !isCharacter(content[strStart+4])) {
                    strEnd += 3;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),TRUE));
                }
                else {
                    while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;    
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                }
            }
            else if(content[strStart] == 'w') {
                if(content[strStart+1] == 'h' && content[strStart+2] == 'i' &&
                   content[strStart+3] == 'l' && content[strStart+4] == 'e' &&
                   !isCharacter(content[strStart+5])) {
                    strEnd += 4;
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),WHILE));
                }
                else {
                    while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;    
                    tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
                }
            }
            else {
                while(isCharacter(content[strEnd+1]) || content[strEnd+1] == '_' || isDigit(content[strEnd+1])) strEnd++;     
                tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),ID));
            }
            strStart = ++strEnd;
        }
        else {
            tokens.push_back(*makeToken(content.substr(strStart,strEnd-strStart+1),UNKNOWN));
            strStart = ++strEnd;
        }
    }
    return strEnd;
}

bool test4Tokenizer(string contentPath) {
    lexToTokens(contentPath, tokens);
    printf("-------------------------\n");
    printf("|       Tokenizer       |\n");
    printf("-------------------------\n");
    for(Token token : tokens) {
        printf("<@%3d:%3d: '%s', Line %d, %d:%d, %s>",
                token.start,token.end,token.lex.c_str(),token.curLine,
                token.curStart,token.curEnd,tokenList[token.type].c_str());
        if(token.type == UNKNOWN) {
            printf("*** !!!Error type!!! ***");
            cout << endl;
            return false;
        }
        cout << endl;
    }
    return true;
}