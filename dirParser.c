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
 * returns number of folders along the path given
 * 
 * @param path Path for which depth is required
 * 
 * @return depth of folder/file
 */
static int getPathDepth (char *path)
{
    int depth = 0;
    for(;path && *path; path++)
        if(*path == PATH_SEP_CHR && *(path+1)) /* ignore the final / or \ */
            depth++;
    return depth;
}

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
    }
    len += strlen (f->baseName);
    name = calloc (len+2, sizeof(char));
    if(f->dirName)
        sprintf (name, "%s%c%s", f->dirName, '/', f->baseName);
    else if (f->parent)
        sprintf (name, "%s%c%s", f->parent->baseName, '/', f->baseName);
    else if (f->baseName)
        sprintf (name, "%s", f->baseName);
    
    return(name);
}

static void* concatArrays (
    void **src,
    int *nSrc,
    void *dst,
    int nDst,
    int eSize
    )
{
}

static int getDirContents(
    char *name,
    char *pat[2],
    char *antiPat[2],
    void*(*callback)(void*, FS_Object *),
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
        FS_Object f;
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
parseDir(
    my_string *paths,
    int numPaths,
    char *pat[2],
    char *antiPat[2],
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
        (*pDirs)[i]->depth = getPathDepth (paths[i]);
        memcpy ((*pDirs)[i]->baseName, paths[i], len);
    }
    if(*nSubDirs == 0)    *nSubDirs = numPaths;

    for(i=0; i<numPaths; i++)
    {
        char *name = getFullName ((*pDirs)[i]);
        FS_Object **dirs = NULL;
        FS_Object **files = NULL;
        int lnFiles = 0;
        int lnSubDirs = 0;
        getDirContents (name, pat, antiPat, callback, &lnFiles,
                        &lnSubDirs, &dirs, &files, cbArgs, (*pDirs)[i]);

        *pDirs = realloc (*pDirs, (lnSubDirs + *nSubDirs)*sizeof(FS_Object*));
        *pFiles = realloc (*pFiles, (lnFiles + *nFiles)*sizeof(FS_Object*));
        
        memcpy (&(*pDirs)[*nSubDirs], dirs, lnSubDirs * sizeof(FS_Object*));
        memcpy (&(*pFiles)[*nFiles], files, lnFiles * sizeof(FS_Object*));
        *nSubDirs += lnSubDirs;
        *nFiles   += lnFiles;

        CHK_FREE (dirs);
        CHK_FREE (files);
        CHK_FREE (name);
        
        if(doDFS)
        {
            int j;
            for(j=0; j<lnSubDirs; j++)
            {
                char *name = getFullName ((*pDirs)[j+*nSubDirs]);
                parseDir (&name, 1, pat, antiPat, callback, nFiles,
                          nSubDirs, pDirs, pFiles, cbArgs, doDFS);
                CHK_FREE (name);
            }
        }
        else if(!doDFS)
        {
            numPaths += lnSubDirs;
        }
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
    parseDir(argv+1, argc-1, NULL, NULL, NULL, &nFiles, &nSubDirs, &dirs,
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
