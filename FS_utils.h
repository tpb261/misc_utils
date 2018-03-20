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
struct _FS_Object_* getToNthPaent (struct _FS_Object_ *F, int n, char **name);

/**
 * Get the full path and filename in one string
 *
 * @param F  FS_Object whose name is required
 *
 * @return full name string
 */
char *getFullName (struct _FS_Object_ *f);

void* printer (void* fmt, struct _FS_Object_ *f, void (*printUserData)(void*));

void sortByTokens ( struct _FS_Object_ **pObjs, int nObjs, int *sortFlds, int *sortOrd, int nSortFlds);

struct _FS_Object_* fileToFSObjs ( char *filename);

int getFSTokens( struct _FS_Object_ *fs, void *vargs);

int getAllFSTokens( struct _FS_Object_ *fs, void *vargs);

int getFileTokens(struct _FS_Object_ *fs, void *vargs);

void freeFields (struct _FS_Object_ *o);

void freeFS_Objects (struct _FS_Object_ **args, int nArgs);

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
