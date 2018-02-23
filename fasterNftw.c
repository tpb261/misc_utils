/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>

	File: fasterNftw.c
	Author: TPB (Halfwit genius)

*/
#include <fasterNftw.h>
#include <strHelper.h>

#if 1//USE_MT_MODE
#include <pthread.h>
#else
void* pthread_create (
    void *dummy,
    void *thr_args,
    void* (*threadFunc)(void *),
    void *thrArgs)
{
    return threadFunc (thrArgs);
}

#define  pthread_join(a, b)
#endif

#if 0
static void* concatArrays (
    void **src,
    int *nSrc,
    void *dst,
    int nDst,
    int size
    )
{
    *src = realloc (*src, (*nSrc+nDst)*size);
    memcpy (&(*src)[*nSrc], dst, nDst*size);
    *nSrc += nDst;
    return *src;
}
#endif

static int getDirContents(
    char  *name,
    char **pat,
    char **antiPat,
    fpNftwCb callback,
    int *nFiles,
    int *nSubDirs,
    FS_Object ***pDirs,
    FS_Object ***pFiles,
    void *cbArgs,
    FS_Object *parent
    )
{
    struct dirent *ent;
    DIR *dir;
    FS_Object ***ptr;
    int pathLen = strlen (name);
    int num = 0;
    void *result = NULL;

    *pDirs = NULL;
    *pFiles = NULL;
    if((dir = opendir (name)) ==  NULL)
    {
        printf ("opendir(%s) failed with errno: %d\n", name, errno);
        return -1;
    }
    while((ent = readdir (dir)) != NULL)
    {
        int entNameLen = strlen (ent->d_name);
        int filePathen = pathLen + entNameLen;
        FS_Object f = {0,};
        int i = 0;
        if ( ent->d_name[0] == '.')
            if (ent->d_name[1]==0
                || ( ent->d_name[1]=='.' && ent->d_name[2]==0 ))
                continue;

        i = (ent->d_type == DT_DIR?1:0);
        if((antiPat && antiPat[i] && strcasestr (ent->d_name, antiPat[i]))
           || (pat && pat[i]  && !strcasestr (ent->d_name, pat[i])))
            continue;

        memset (&f, 0, sizeof(f));
        f.parent = parent;
        f.baseName   = calloc (entNameLen+1, sizeof(char));
        f.depth  = parent->depth+1;
        memcpy (f.baseName, ent->d_name, entNameLen+1);
        if(callback) callback (&f, cbArgs);
        if( ent->d_type == DT_DIR)
        {
            num = ++(*nSubDirs);
            ptr = pDirs;
            parent->numSubDirs++;
            f.dirName = getFullName (parent);
        }
	else if( ent->d_type == DT_REG)
        {
            num = ++(*nFiles);
            parent->numFiles++;
            ptr = pFiles;
        }
        *ptr = realloc (*ptr, num*sizeof(FS_Object*));
        (*ptr)[num-1] = (FS_Object*)calloc (sizeof(FS_Object), 1);
        memcpy ((*ptr)[num-1], &f, sizeof(FS_Object));
    }
    closedir (dir);
}

static void* getDirContentsThr(void *args)
{
    dirContent_t   *p          = (dirContent_t *)args;
    char           *name       = p->name;
    char          **pat        = p->pat;
    char          **antiPat    = p->antiPat;
    fpNftwCb        callback   = p->callback;
    int            *nFiles     = p->nFiles;
    int            *nSubDirs   = p->nSubDirs;
    FS_Object    ***pDirs      = p->pDirs;
    FS_Object    ***pFiles     = p->pFiles;
    void           *cbArgs     = p->cbArgs;
    FS_Object      *parent     = p->parent;

    getDirContents (name, pat, antiPat, callback, nFiles, nSubDirs, pDirs,
                    pFiles, cbArgs, parent);

}

/**
 * Parse given directories recursively. Popuate the dirs and files arrays
 * and the count of subdirs and files. Also, run callbacks if provided on
 * the files and subdirs. Default implementation is BFS. Can run DFS too.
 *
 * @param paths        directory to parse
 * @param numPaths     number of paths input
 * @paths pat          patterns to include [0] for filename, [1] for directory
 * @paths antiPat      patterns to exclude [0] for filename, [1] for directory
 * @param callback     callback to call for both files and directories
 * @param numFiles     number of files found
 * @param numSubDirs   number of subdirs parsed
 * @param dirs         list of directories parsed - order as returned by OS
 * @param files        list of files parsed - orider as returned by OS
 * @param cbArgs       arguments for callback
 * @param doDFS        drill down directories as we find them
 */
void
fasterNftw(
    my_string *paths,
    int numPaths,
    char **pat,
    char **antiPat,
    void*(*callback)(void*, FS_Object *),
    int *nFiles,
    int *nSubDirs,
    FS_Object ***pDirs,
    FS_Object ***pFiles,
    void *cbArgs,
    int doDFS
    )
{
    DIR *dir;
    FS_Object ***ptr;
    int num;
    int n = numPaths;
    int i;
    int numThreads = 1;

    if(pDirs)
    {
        *pDirs = realloc (*pDirs, n*sizeof(FS_Object*));
    }
    if(*pFiles)
    {
        *pFiles = realloc (*pFiles, numPaths*sizeof(FS_Object*));
    }
    for(i = 0; i<numPaths; i++)
    {
        int len = strlen (paths[i]);
        (*pDirs)[i] = (FS_Object*)calloc (sizeof(FS_Object), 1);
        if(paths[i][len-1] == PATH_SEP_CHR) len--;
        (*pDirs)[i]->baseName = calloc (1+len, sizeof(char));
        (*pDirs)[i]->depth = getNumTokens (paths[i], PATH_SEP_CHR);
        memcpy ((*pDirs)[i]->baseName, paths[i], len);
        if(callback) callback ((*pDirs)[i], cbArgs);
    }
    if(*nSubDirs == 0)    *nSubDirs = numPaths;

    for(i=0; i<numPaths; )
    {
        char **name        = calloc (sizeof(char*), numThreads);
        FS_Object ***dirs  = calloc (sizeof(FS_Object **), numThreads);
        FS_Object ***files = calloc (sizeof(FS_Object **), numThreads);
        int *lnFiles       = calloc (sizeof(int), numThreads);
        int *lnSubDirs     = calloc (sizeof(int), numThreads);
        int t;
        int numSubDirs = *nSubDirs;
        int numFiles   = *nFiles;
        int tmpNPaths  = numPaths;
        pthread_t *thrs = calloc (numThreads, sizeof(pthread_t));

        for(t = 0; t<numThreads && i+t<numPaths; t++)
        {
            dirContent_t s = {NULL, pat, antiPat, callback, lnFiles+t,
                              lnSubDirs+t, dirs+t, files+t, cbArgs,
                              (*pDirs)[i+t], t};
            name[t] = getFullName ((*pDirs)[i+t]);
            s.name = name[t];
            pthread_create (&thrs[t], NULL, getDirContentsThr, &s);
//            getDirContentsThr (&s);
        }
        for(t = 0; t<numThreads && i+t<numPaths; t++)
        {
            pthread_join (thrs[t], NULL);
        }
        /* join here */
        for(t = 0; t<numThreads && i+t<numPaths; t++)
        {
            numSubDirs += lnSubDirs[t];
            numFiles   += lnFiles[t];
        }
        *pDirs = realloc (*pDirs, (numSubDirs*sizeof(FS_Object*)));
        *pFiles = realloc (*pFiles, (numFiles)*sizeof(FS_Object*));
        for(t = 0; t<numThreads && i<numPaths; t++, i++)
        {
            memcpy (*pDirs+*nSubDirs, dirs[t], lnSubDirs[t] * sizeof(FS_Object*));
            memcpy (*pFiles+*nFiles, files[t], lnFiles[t] * sizeof(FS_Object*));
            *nSubDirs += lnSubDirs[t];
            *nFiles   += lnFiles[t];
            if(doDFS)
            {
                int j;
                for(j=0; j<lnSubDirs[t]; j++)
                {
                    char *name = getFullName ((*pDirs)[j+*nSubDirs]);
                    fasterNftw (&name, 1, pat, antiPat, callback, nFiles,
                              nSubDirs, pDirs, pFiles, cbArgs, doDFS);
                    CHK_FREE (name);
                }
            }
            else if(!doDFS)
            {
                tmpNPaths += lnSubDirs[t];
            }
        }
        for(t=0; t<numThreads; t++)
        {
            CHK_FREE (name[t]);
            CHK_FREE (dirs[t]);
            CHK_FREE (files[t]);
        }
        CHK_FREE (name);
        CHK_FREE (dirs);
        CHK_FREE (files);
        CHK_FREE (lnFiles);
        CHK_FREE (lnSubDirs);
        CHK_FREE (thrs);

        if(!doDFS) numPaths = tmpNPaths;
    }
}

