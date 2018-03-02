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
 * returns number of tokens in string
 * 
 * @param path String to tokenize
 * 
 * @return number of tokens
 */
int getNumTokens (char *path, char *tokens)
{
    int depth = 0;//(path && (NULL=strchr (*path, tokens)))?1:0;

    if(!path) return 0;
    
    while(*path)
    {
        /* keep moving as long as we see characters from the tokens */
        while(*path && strchr (tokens, *path))
            path++;
        if(*path) depth++;
        /* keep moving as long as we see non-tokenizing characters */
        while(*path && !strchr (tokens, *path))
            path++;
    }
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
int getTokens (char *path, char ***fields, char *tokens)
{
    int numTokens = getNumTokens (path, tokens);
    int i = 0;
    int j = 0;
    int k = 0;
    *fields = calloc (numTokens, sizeof(char*));

    if(strchr (path, '_'))
    {
        printf ("I'm here\n");
    }
    
    for(i=0; i<numTokens; i++)
    {
        /* keep moving as long as we see non-tokenizing characters */
        for(k=j; path[j] && !strchr (tokens, path[j]); j++);
        str_dbg_printf ("%s:%d %s \n", __FUNCTION__, __LINE__, path+j);

        (*fields)[i] = calloc (j-k+1, sizeof(char));
        memcpy ((*fields)[i], path+k, j-k);

        /* keep moving as long as we see tokenizing characters */
        for(; path[j] && strchr (tokens, path[j]); j++);
        str_dbg_printf ("%s:%d %s %s\n", __FUNCTION__, __LINE__, (*fields)[i], path+j);
    }
    str_dbg_printf ("numTokens = %d\n", numTokens);
    return numTokens;
}
