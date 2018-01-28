#include <stdlib.h>
#include <string.h>
/** 
 * returns number of folders along the path given
 * 
 * @param path Path for which depth is required
 * 
 * @return depth of folder/file
 */
int getNumTokens (char *path, char token)
{
    int depth = 0;
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
int getTokens (char *path, char **tokens, char token)
{
    int numTokens = getNumTokens (path, token);
    int i = 0;
    int j = 0;
    tokens = calloc (numTokens, sizeof(char*));
    for(i=0; i<numTokens; i++)
    {
        tokens[i] = path+j;
        for(j; path[j] && path[j]!=token; j++);
    }
}

