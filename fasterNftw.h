#ifndef FASTER_NFTW_H
#define FASTER_NFTW_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <dirent.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <strHelper.h>
//#include <FS_utils.h>
#include <array.h>

typedef struct _FS_Object_
{
    char *baseName;
    char *dirName;
    struct _FS_Object_ *parent;
    int depth;
    int numFiles;
    int numSubDirs;
    void **userData;
}FS_Object;

typedef int(*fpNftwCb)(FS_Object *, void *);

typedef struct _direContent_t
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
    int             threadIdx;
}dirContent_t;

char *getFullName (FS_Object *f);

#define PATH_SEP_CHR '/'
#define PATH_SEP_STR "/"

void* printer (void* fmt, FS_Object *f, void (*printUserData)(void*));

void
fasterNftw(
    my_string *paths,
    int numPaths,
    char **pat,
    char **antiPat,
    fpNftwCb callback,
    int *nFiles,
    int *nSubDirs,
    FS_Object ***pDirs,
    FS_Object ***pFiles,
    void *cbArgs,
    int doDFS
    );
#endif
