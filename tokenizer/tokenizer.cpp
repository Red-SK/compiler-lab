#include "tokenizer.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>
#include <sstream>

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
    cout << path << ": " << fileSize << " Bytes." << endl;
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
    int start = 0; 
    int end = 0;
    while(content[start] != '\0') {
        // skip blank space
        if(content[start] == ' ' || content[start] == '\n' || content[start] == '\r') {
            start = ++end;
        }
        else if(content[start] == '+') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),ADD));
            start = ++end;
        }
        else if(content[start] == '-') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),SUB));
            start = ++end;
        }
        else if(content[start] == '*') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),MUL));
            start = ++end;
        }
        else if(content[start] == '/') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),DIV));
            start = ++end;
        }
        else if(content[start] == '(') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),CIR_LEFT_BRACKET));
            start = ++end;
        }
        else if(content[start] == ')') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),CIR_RIGHT_BRACKET));
            start = ++end;
        }
        else if(content[start] == '[') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),SQ_LEFT_BRACKET));
            start = ++end;
        }
        else if(content[start] == ']') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),SQ_RIGHT_BRACKET));
            start = ++end;
        }
        else if(content[start] == '{') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),LEFT_BLOCK));
            start = ++end;
        }
        else if(content[start] == '}') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),RIGHT_BLOCK));
            start = ++end;
        }
        else if(content[start] == '<') {
            if(content[start+1] == '=') {
                end++;
                tokens.push_back(*makeToken(content.substr(start,end-start+1),LESS_EQUAL));              
                start = ++end;
            } else {
                tokens.push_back(*makeToken(content.substr(start,end-start+1),LESS));
                start = ++end;
            }          
        }
        else if(content[start] == '>') {
            if(content[start+1] == '=') {
                end++;
                tokens.push_back(*makeToken(content.substr(start,end-start+1),GREATER_EQUAL));              
                start = ++end;
            } else {
                tokens.push_back(*makeToken(content.substr(start,end-start+1),GREATER));
                start = ++end;
            }          
        }
        else if(content[start] == '=') {
            if(content[start+1] == '=') {
                end++;
                tokens.push_back(*makeToken(content.substr(start,end-start+1),EQUAL));                
                start = ++end;
            } else {
                tokens.push_back(*makeToken(content.substr(start,end-start+1),ASSIGN));
                start = ++end;
            }          
        }
        else if(content[start] == '!') {
            if(content[start+1] == '=') {
                end++;
                tokens.push_back(*makeToken(content.substr(start,end-start+1),NOT_EQUAL));              
                start = ++end;
            } else {
                tokens.push_back(*makeToken(content.substr(start,end-start+1),NOT));
                start = ++end;
            }      
        }
        else if(content[start] == '&') {
            if(content[start+1] == '&') {
                end++;
                tokens.push_back(*makeToken(content.substr(start,end-start+1),AND));                
                start = ++end;
            } else {

            }
        }
        else if(content[start] == '|') {
            if(content[start+1] == '|') {
                end++;
                tokens.push_back(*makeToken(content.substr(start,end-start+1),OR));                
                start = ++end;
            } else {
                
            }
        }
        else if(content[start] == ';') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),SEMICOLON));
            start = ++end;
        }
        else if(content[start] == ',') {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),COMMA));
            start = ++end;
        }
        // 数字
        else if(isDigit(content[start])) {
            while(isDigit(content[end])) end++;
            if(content[end] == '.') { // 实数
                end++;
                while(isDigit(content[end])) end++;
                tokens.push_back(*makeToken(content.substr(start,end-start),REAL_NUM));
            } else { // 整数
                tokens.push_back(*makeToken(content.substr(start,end-start),INT_NUM));
            }
            start = end;
        }
        // 关键字和id
        else if(isCharacter(content[start])) {
            /* 关键字 */
            if(content[start] == 'b') {
                if(content[start+1] == 'o' && content[start+2] == 'o' && 
                   content[start+3] == 'l' && !isCharacter(content[start+4])) {
                    end += 3;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),BOOL));
                }
                else if(content[start+1] == 'r' && content[start+2] == 'e' &&
                        content[start+3] == 'a' && content[start+4] == 'k' &&
                        !isCharacter(content[start+5])) {
                    end += 4;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),BREAK));
                }
                else {
                    while(isCharacter(content[end+1]) || content[end+1] == '_') end++;                      
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
                }
            }
            else if(content[start] == 'i') {
                // int
                if(content[start+1] == 'n' && content[start+2] == 't' &&
                   !isCharacter(content[start+3])) {
                    end += 2;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),INT));
                }
                // if --> if()
                else if(content[start+1] == 'f' && !isCharacter(content[start+2])) {
                    end += 1;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),IF));
                }
                else {
                    while(isCharacter(content[end+1]) || content[end+1] == '_') end++;   
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
                }
            }
            else if(content[start] == 'e') {
                if(content[start+1] == 'l' && content[start+2] == 's' &&
                   content[start+3] == 'e' && !isCharacter(content[start+4])) {
                    end += 3;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),ELSE));
                }
                else {
                    while(isCharacter(content[end+1]) || content[end+1] == '_') end++;   
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
                }
            }
            else if(content[start] == 'f') {
                if(content[start+1] == 'a' && content[start+2] == 'l' &&
                   content[start+3] == 's' && content[start+4] == 'e' && 
                   !isCharacter(content[start+5])) {
                    end += 4;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),FALSE));
                }
                else if(content[start+1] == 'o' && content[start+2] == 'r' &&
                       !isCharacter(content[start+3])) {
                    end += 2;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),FOR));
                }
                else {
                    while(isCharacter(content[end+1]) || content[end+1] == '_') end++;   
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
                }
            }
            else if(content[start] == 'r') {
                if(content[start+1] == 'e') {
                    if(content[start+2] == 'a' && content[start+3] == 'l' &&
                       !isCharacter(content[start+4])) {
                        end += 3;
                        tokens.push_back(*makeToken(content.substr(start,end-start+1),REAL));
                    }
                    else if(content[start+2] == 't' && content[start+3] == 'u' &&
                       content[start+4] == 'r' && content[start+5] == 'n' &&
                       !isCharacter(content[start+5])) {
                        end += 5;
                        tokens.push_back(*makeToken(content.substr(start,end-start+1),RETURN));
                    }
                    else {
                        while(isCharacter(content[end+1]) || content[end+1] == '_') end++;    
                        tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
                    }
                }
                else {
                    while(isCharacter(content[end+1]) || content[end+1] == '_') end++;    
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
                }
            }
            else if(content[start] == 't') {
                if(content[start+1] == 'r' && content[start+2] == 'u' &&
                   content[start+3] == 'e' && !isCharacter(content[start+4])) {
                    end += 3;
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),TRUE));
                }
                else {
                    while(isCharacter(content[end+1]) || content[end+1] == '_') end++;    
                    tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
                }
            }
            else {
                while(isCharacter(content[end+1]) || content[end+1] == '_') end++;     
                tokens.push_back(*makeToken(content.substr(start,end-start+1),ID));
            }
            start = ++end;
        }
        else {
            tokens.push_back(*makeToken(content.substr(start,end-start+1),UNKNOWN));
            start = ++end;
        }
    }
    return end;
}