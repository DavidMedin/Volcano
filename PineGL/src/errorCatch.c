#include "errorCatch.h"

void __Error(const char* file,const long line,const char* string,...){
    char* buff = malloc(sizeof(char)*50);
    va_list args;
    va_start(args,string);
    vsprintf_s(buff,50,string,args);
    printf("%s:%d -> %s\n",file,line,buff);
    free(buff);
    #ifdef ENABLE_BREAKPOINTS
    __debugbreak();
    #endif
}