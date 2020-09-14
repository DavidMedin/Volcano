#include "errorCatch.h"

void __Error(const char* file,const long line,const char* string,...){
    va_list args;
    va_start(args,string);
    printf("%s:%d -> ",file,line);
    vprintf(string,args);
}