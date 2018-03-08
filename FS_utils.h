#ifndef _FS_UTILS_H_
#define _FS_UTILS_H_
#include <strHelper.h>
#include <fasterNftw.h>

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

void * getFSTokens( FS_Object *fs, void *vargs);

void * getAllFSTokens( FS_Object *fs, void *vargs);

void * getFileTokens(FS_Object *fs, void *vargs);

void freeFS_Objects (FS_Object **args, int nArgs);

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
