#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strHelper.h>

#if STR_DBG_PRINTF
#define str_dbg_printf printf
#else
#define str_dbg_printf(...)
#endif

/** 
 * returns number of folders along the path given
 * 
 * @param path Path for which depth is required
 * 
 * @return depth of folder/file
 */
int getNumTokens (char *path, char token)
{
    int depth = path && *path?1:0;
    for(;path && *path; path++)
        if(*path == token && *(path+1)) /* ignore the final / or \ */
            depth++;
    return depth;
}

/** 
 * Tokenize a string into tokens
 * 
 * @param path    string to be tokenized
 * @param tokens  pointers to the tokens (picked from the string)
 * @param token   token to use to split the string
 * 
 * @return        number of tokens
 */
int getTokens (char *path, char ***tokens, char token)
{
    int numTokens = getNumTokens (path, token);
    int i = 0;
    int j = 0;
    int k = 0;
    *tokens = calloc (numTokens, sizeof(char*));
    for(i=0; i<numTokens; i++, j++)
    {
        for(k=j; path[j] && path[j]!=token; j++);
        str_dbg_printf ("%s:%d %s \n", __FUNCTION__, __LINE__, path+j);
        (*tokens)[i] = calloc (j-k+1, sizeof(char));
        memcpy ((*tokens)[i], path+k, j-k);
        str_dbg_printf ("%s:%d %s %s\n", __FUNCTION__, __LINE__, (*tokens)[i], path+k);
    }
    str_dbg_printf ("numTokens = %d\n", numTokens);
    return numTokens;
}

