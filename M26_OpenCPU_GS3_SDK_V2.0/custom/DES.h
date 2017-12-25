#ifndef DES_H  
#define DES_H  
#include <stdbool.h>

enum {encrypt,decrypt};//
void des_run(char out[8],char in[8],_Bool type);//type=encrypt
//
void des_setkey(const char key[8]);

#endif
