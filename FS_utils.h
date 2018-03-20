#ifndef _FS_UTILS_H_
#define _FS_UTILS_H_
#include <strHelper.h>
#include <fasterNftw.h>
#include <array.h>

typedef struct _pathFields_t
{
    int numFields;
    array impFields;
}pathFields;

typedef struct _fields_t_
{
    int idx;
    char *field;
}fields_t;

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
FS_Object* getToNthPaent (FS_Object *F, int n, char **name);

/**
 * Get the full path and filename in one string
 *
 * @param F  FS_Object whose name is required
 *
 * @return full name string
 */
char *getFullName (FS_Object *f);


void* printer (void* fmt, FS_Object *f, void (*printUserData)(void*));

void sortByTokens ( FS_Object **pObjs, int nObjs, int *sortFlds, int *sortOrd, int nSortFlds);

FS_Object* fileToFSObjs ( char *filename);

int getFSTokens(FS_Object *fs, void *vargs);

int getAllFSTokens(FS_Object *fs, void *vargs);

int getFileTokens(FS_Object *fs, void *vargs);

void freeFields (FS_Object *o);

void freeFS_Objects (FS_Object **args, int nArgs);

int getIndexOfField (FS_Object *o1, int fieldIdx);


#define GET_FIELDS_PTR_FSO(o, f)                                \
    do{                                                         \
        if(o && o->userData)                                    \
        {                                                       \
            pathFields *ptr = *((pathFields**)(o->userData));   \
            if(ptr)                                             \
            {                                                   \
                array fld = ptr->impFields;                     \
                f  = (fields_t *)fld.data;                      \
            }                                                   \
        }                                                       \
    }while(0)

#define GET_PATHFIELDS_PTR_FSO(o , a)                           \
    do{                                                         \
        if(o && o->userData)                                    \
        {                                                       \
            pathFields *ptr = *((pathFields**)(o->userData));   \
            if(ptr)                                             \
            {                                                   \
                a = ptr->impFields;                             \
            }                                                   \
        }                                                       \
    }while(0)


#endif
