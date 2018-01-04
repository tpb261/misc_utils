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

/** 
 * 
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
    free (name1);
    return(name);
}

int
parseDir(
    my_string *paths,
    int numPaths,
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
    struct dirent *ent;
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
        int pathLen = strlen (name);
        if((dir = opendir (name)) !=  NULL)
        {
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

                memset (&f, 0, sizeof(f));
                f.name = calloc (entNameLen+1, sizeof(char));
                f.parent = (*pDirs)[i];
                f.type = ent->d_type;
                f.depth = f.parent->depth+1;
                memcpy (f.name, ent->d_name, entNameLen+1);
                entFullName = calloc (filePathen +entNameLen + 2, sizeof(char));
                sprintf (entFullName, "%s/%s", name, ent->d_name);

                switch(ent->d_type)
                {
                case DT_DIR:
                {
                    if (cbDir) cbDir (dirArgs, &f);
                    if (cbCommon) cbCommon (comArgs, &f);
                    num = ++(*nSubDirs);
                    ptr = pDirs;
                    (*pDirs)[i]->numSubDirs++;
                    break;
                }
                case DT_REG:
                {
                    if (cbFile) cbFile (fileArgs, &f);
                    if (cbCommon) cbCommon (comArgs, &f);
                    num = ++(*nFiles);
                    (*pDirs)[i]->numFiles++;
                    ptr = pFiles;
                    break;
                }
                default:
                    continue;
                }

                *ptr = realloc (*ptr, num*sizeof(FS_Object*));
                (*ptr)[num-1] = (FS_Object*)calloc (sizeof(FS_Object), 1);
                memcpy ((*ptr)[num-1], &f, sizeof(FS_Object));
                if(ent->d_type == DT_DIR && doDFS)
                {
                    parseDir ((char**)&ent->d_name, 1, cbFile, cbDir, cbCommon,
                              nFiles, nSubDirs, pDirs, pFiles, dirArgs, fileArgs,
                              comArgs, doDFS);
                }
                else if(ent->d_type == DT_DIR && !doDFS)
                {
                    numPaths++;
                }
                free (entFullName);
            }
            closedir (dir);
        }
        else
        {
            printf ("opendir(%s) failed with errno: %d\n", (*pDirs)[i]->name, errno);
        }
        free (name);
    }
    return 0;
}

void* printer (void* fmt, FS_Object *f)
{
    if(f)
    {
        char *name = getFullName (f);
        printf ("%s %d %s:\n", f->type==DT_DIR?"dir":"file", f->depth, name);
        free (name);
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
        free (dirs[i]->name);
        free (dirs[i]);
    }

    for(i = 0; i<nFiles; i++)
    {
        printer (NULL, files[i]);
        free (files[i]->name);
        free (files[i]);
    }
    free (dirs);
    free (files);
}
