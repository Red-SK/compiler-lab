#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace std;

// 类型重定义
typedef unsigned int uint; 
typedef unsigned long long int64;
typedef double real64;

vector<string> split(const string& str, const string& delim);
bool isZero(double num);

#endif