#ifndef _STR_HELPER_H
#define _STR_HELPER_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef char* my_string ;

#define CHK_FREE(p) if(p) free (p)

/** 
 * returns number of folders along the path given
 * 
 * @param path Path for which depth is required
 * 
 * @return depth of folder/file
 */
int getNumTokens (char *path, char *token);

/** 
 * Tokenize a string into tokens
 * 
 * @param path    string to be tokenized
 * @param tokens  pointers to the tokens (picked from the string)
 * @param token   token to use to split the string
 * 
 * @return        number of tokens
 */
int getTokens (char *path, char ***tokens, char *token);

char* strcasestr (const char *hayStack, const char *needle);

#endif
