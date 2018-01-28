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

	File: pgmIo.c
	Author: TPB (Halfwit genius)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

typedef char* my_string;

#define PATH_SEP_CHR '/'
#define PATH_SEP_STR "/"

typedef struct _FS_Object_
{
    struct _FS_Object_ *parent;
    char *baseName;
    char *dirName; /*< we will not fill this for files to reduce memory usage */
    int depth;
    int numSubDirs;
    int numFiles;
}FS_Object;

typedef void*(*fpNftwCb)(void*, FS_Object *);

typedef struct _dirContent_s
{
    char           *name;
    char          **pat;
    char          **antiPat;
    fpNftwCb        callback;
    int            *nFiles;
    int            *nSubDirs;
    FS_Object    ***pDirs;
    FS_Object    ***pFiles;
    void           *cbArgs;
    FS_Object      *parent;
}dirContent_t;

#define CHK_FREE(p) if(p) free (p)

#if 0
/**
 * Get the nth-level parent of the given FS object
 * Also, sets the name, from that parent till this object's parent
 *
 * @param F     object whose parent is required
 * @param n     nth parent
 * @param name  name from nth-parne to 1st parent
 *
 * @return FS_object of the n-th parent
 */
FS_Object* getToNthPaent (FS_Object *F, int n, char **name)
{
    int l;
    FS_Object *f = F->parent;
    int len = 0;
    char *name1 = NULL;

    for(l = 1; l<1 && f; l++, f=f->parent)
        len += strlen (f->baseName);
    len += strlen (f->baseName);

    *name = realloc (*name, len*sizeof(char));
    name1 = realloc (name1, len*sizeof(char));

    for(f=F->parent, l=1; l<n && f; f=f->parent)
    {
        strcpy (name1, *name);
        sprintf (*name, "%s/%s", f->baseName, name1);
    }
    CHK_FREE (name1);
    return f;
}
#endif

/**
 * Get the full path and filename in one string
 *
 * @param F  FS_Object whose name is required
 *
 * @return full name string
 */
char *getFullName (FS_Object *f)
{
    char *name = NULL;
    int len = 0;
    int len1 = 0;
    if(!f)
        return NULL;
    if(f->dirName != NULL)
    {
        len = strlen (f->dirName);
    }
    else if(f->parent)
    {
        len = strlen (f->parent->baseName);
        if(f->parent->dirName)
        {
            len += strlen (f->parent->dirName);
            len++;
        }
    }
    len += strlen (f->baseName);
    name = calloc (len+2, sizeof(char));
    if(f->dirName)
        sprintf (name, "%s%c%s", f->dirName, PATH_SEP_CHR, f->baseName);
    else if (f->parent && f->parent->dirName)
        sprintf (name, "%s%c%s%c%s", f->parent->dirName, PATH_SEP_CHR,
                 f->parent->baseName,PATH_SEP_CHR, f->baseName);
    else if (f->parent)
        sprintf (name, "%s%c%s", f->parent->baseName, PATH_SEP_CHR,
                 f->baseName);
    else if (f->baseName)
        sprintf (name, "%s", f->baseName);

    return(name);
}

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
            if (ent->d_name[1]==0 )
                continue;
            else if( ent->d_name[1]=='.' && ent->d_name[2]==0 )
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
        if(callback) callback (cbArgs, &f);
        switch(ent->d_type)
        {
        case DT_DIR:
        {
            num = ++(*nSubDirs);
            ptr = pDirs;
            parent->numSubDirs++;
            f.dirName = getFullName (parent);
            break;
        }
        case DT_REG:
        {
            num = ++(*nFiles);
            parent->numFiles++;
            ptr = pFiles;
            break;
        }
        default:
            continue;
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

        for(t = 0; t<numThreads && i+t<numPaths; t++)
        {
            dirContent_t s = {NULL, pat, antiPat, callback, lnFiles+t,
                              lnSubDirs+t, dirs+t, files+t, cbArgs,
                              (*pDirs)[i+t]};
            name[t] = getFullName ((*pDirs)[i+t]);
            s.name = name[t];
            getDirContentsThr (&s);
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

        if(!doDFS) numPaths = tmpNPaths;
    }
}

void* printer (void* fmt, FS_Object *f)
{
    char **args = (char**)fmt;
    if(f)
    {
        char *name = getFullName (f);
        printf ("%s %d %s:\n", f->dirName?"dir":"file", f->depth, name);
        CHK_FREE (name);
    }
    return NULL;

}

#if 1//UNIT_TESTS
int main (int argc, char *argv[])
{
    FS_Object **dirs = NULL;
    FS_Object **files = NULL;
    int i;
    int nFiles = 0;
    int nSubDirs = 0;
    char *args[] = {"dir: %s\n", "file: %s\n"};
    char *pat[2] = {NULL, NULL};
    char *antiPat[2] = {NULL, NULL};
    fasterNftw(argv+1, argc-1, NULL, NULL, NULL, &nFiles, &nSubDirs, &dirs,
             &files, (void*)args, 0);

    for(i = 0; i<nSubDirs; i++)
    {
        printer (NULL, dirs[i]);
    }

    for(i = 0; i<nFiles; i++)
    {
        printer (NULL, files[i]);
    }

    for(i=0;i<nSubDirs; i++)
    {
        CHK_FREE (dirs[i]->baseName);
        CHK_FREE (dirs[i]);
    }

    for(i = 0; i<nFiles; i++)
    {
        CHK_FREE (files[i]->baseName);
        CHK_FREE (files[i]);
    }

    CHK_FREE (dirs);
    CHK_FREE (files);
}
#endif
