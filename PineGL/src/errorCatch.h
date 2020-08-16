#pragma once
// #include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define ENABLE_BREAKPOINTS
#define Error(string,...) __Error(__FILE__,__LINE__,string,__VA_ARGS__)

void __Error(const char* file,const long line,const char* string,...);