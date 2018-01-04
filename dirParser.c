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

#define PATH_SEP '/'

typedef struct _FS_Object_
{
    struct _FS_Object_ *parent;
    char *name;
    int depth;
    int type;
    int numSubDirs;
    int numFiles;
}FS_Object;

#define CHK_FREE(p) if(p) free (p)

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
        len += strlen (f->name);
    len += strlen (f->name);

    *name = realloc (*name, len*sizeof(char));
    name1 = realloc (name1, len*sizeof(char));

    for(f=F->parent, l=1; l<n && f; f=f->parent)
    {
        memcpy (name1, name, len+1);
        sprintf (*name, "%s/%s", f->name, name1);
    }
    CHK_FREE (name1);
    return f;
}

/** 
 * Get the full path and filename in one string
 * 
 * @param F  FS_Object whose name is required
 * 
 * @return full name string
 */
char *getFullName (FS_Object *F)
{
    int len = 0;
    int levels = 0;
    FS_Object *f = F;
    char *name = NULL;
    char *name1 = NULL;
    for(levels = 0; f; f=f->parent, levels++)
        len += strlen (f->name);
    
    name = calloc (len+levels+1, sizeof(char));
    name1 = calloc (len+levels+1, sizeof(char));

    sprintf (name, "%s", F->name);
    
    for(f=F->parent; f; f=f->parent)
    {
        memcpy (name1, name, len+1);
        sprintf (name, "%s/%s", f->name, name1);
    }
    CHK_FREE (name1);
    return(name);
}

static int getDirContents(
    char *name,
    char *pattern,
    void*(*cbFile)(void*, FS_Object *),
    void*(*cbDir)(void*, FS_Object *),
    void*(*cbCommon)(void*, FS_Object *),
    int *nFiles,
    int *nSubDirs,
    FS_Object ***pDirs,
    FS_Object ***pFiles,
    void *dirArgs,
    void *fileArgs,
    void *comArgs,
    FS_Object *parent
    )
{
    struct dirent *ent;
    DIR *dir;
    FS_Object ***ptr;
    int pathLen = strlen (name);
    int num = 0;
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
        char *entFullName = NULL;
        FS_Object f;
                
        if ( ent->d_name[0] == '.')
            if (ent->d_name[1]==0 )
                continue;
            else if( ent->d_name[1]=='.' && ent->d_name[2]==0 )
                continue;

        if(!(ent->d_type == DT_REG && strcasestr (ent->d_name, pattern)))
            continue;
        
        memset (&f, 0, sizeof(f));
        f.name   = calloc (entNameLen+1, sizeof(char));
        f.parent = parent;
        f.type   = ent->d_type;
        f.depth  = parent->depth+1;
        memcpy (f.name, ent->d_name, entNameLen+1);
        entFullName = calloc (filePathen +entNameLen + 2, sizeof(char));
        sprintf (entFullName, "%s/%s", name, ent->d_name);

        switch(ent->d_type)
        {
        case DT_DIR:
        {
            if (cbDir) cbDir (dirArgs, &f);
            num = ++(*nSubDirs);
            ptr = pDirs;
            parent->numSubDirs++;
            break;
        }
        case DT_REG:
        {
            if (cbFile) cbFile (fileArgs, &f);
            num = ++(*nFiles);
            parent->numFiles++;
            ptr = pFiles;
            break;
        }
        default:
            continue;
        }
        if (cbCommon) cbCommon (comArgs, &f);
        *ptr = realloc (*ptr, num*sizeof(FS_Object*));
        (*ptr)[num-1] = (FS_Object*)calloc (sizeof(FS_Object), 1);
        memcpy ((*ptr)[num-1], &f, sizeof(FS_Object));
        CHK_FREE (entFullName);
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
 * @param fileCallback callback to call for files
 * @param dirCallback  callback to call for subdirs
 * @param comCallback  callback to call for both files and directories
 * @param numFiles     number of files found
 * @param numSubDirs   number of subdirs parsed
 * @param dirs         list of directories parsed - order as returned by OS
 * @param files        list of files parsed - orider as returned by OS
 * @param dirArgs      arguments for dirCallback
 * @param fileArgs     arguments for fileCallback
 * @param comArgs      arguments for comCallback
 * 
 * @return 
 */
int
parseDir(
    my_string *paths,
    int numPaths,
    char *pattern,
    void*(*cbFile)(void*, FS_Object *),
    void*(*cbDir)(void*, FS_Object *),
    void*(*cbCommon)(void*, FS_Object *),
    int *nFiles,
    int *nSubDirs,
    FS_Object ***pDirs,
    FS_Object ***pFiles,
    void *dirArgs,
    void *fileArgs,
    void *comArgs,
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
        (*pDirs)[i]->type = DT_DIR;
        if(paths[i][len-1] == PATH_SEP) len--;
        (*pDirs)[i]->name = calloc (1+len, sizeof(char));
        memcpy ((*pDirs)[i]->name, paths[i], len);
    }
    if(*nSubDirs == 0)    *nSubDirs = numPaths;

    for(i=0; i<numPaths; i++)
    {
        char *name = getFullName ((*pDirs)[i]);
        FS_Object **dirs = NULL;
        FS_Object **files = NULL;
        int lnFiles = 0;
        int lnSubDirs = 0;
        getDirContents (name, pattern, cbFile, cbDir, cbCommon, &lnFiles,
                        &lnSubDirs, &dirs, &files, dirArgs, fileArgs, comArgs,
                        (*pDirs)[i]);

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
                parseDir (&name, 1, pattern, cbFile, cbDir, cbCommon,
                          nFiles, nSubDirs, pDirs, pFiles, dirArgs, fileArgs,
                          comArgs, doDFS);
                CHK_FREE (name);
            }
        }
        else if(!doDFS)
        {
            numPaths += lnSubDirs;
        }
    }
    return 0;
}

void* printer (void* fmt, FS_Object *f)
{
    if(f)
    {
        char *name = getFullName (f);
        printf ("%s %d %s:\n", f->type==DT_DIR?"dir":"file", f->depth, name);
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
    parseDir(argv+1, argc-1, printer, printer, NULL, &nFiles, &nSubDirs, &dirs,
             &files, "dir: %s\n", "file: %s\n", NULL, 0);

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
        CHK_FREE (dirs[i]->name);
        CHK_FREE (dirs[i]);
    }

    for(i = 0; i<nFiles; i++)
    {
        CHK_FREE (files[i]->name);
        CHK_FREE (files[i]);
    }

    CHK_FREE (dirs);
    CHK_FREE (files);
}
