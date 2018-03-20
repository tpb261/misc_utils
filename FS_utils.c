#include <strHelper.h>
#include <FS_utils.h>
//#include <fasterNftw.h>

void printUserData (void *userData);

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

    for(l = 1; l<1 && f; l++, f=f->parent);
    return f;
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

void* printer (void* fmt, FS_Object *f, void (*printUserData)(void*))
{
    char **args = (char**)fmt;
    if(f)
    {
        char *name = getFullName (f);
        printf ("%s %d %s:\n", f->dirName?"dir":"file", f->depth, name);
        CHK_FREE (name);
    }
    if(printUserData)
        printUserData (f->userData);
    return NULL;

}

FS_Object*
fileToFSObjs (
    char *filename
    )
{
    int fid = -1;
    int i;
    array *pathComps = calloc (sizeof(array), 1);
    char **path = NULL;
    FS_Object *o1 = NULL;
    FS_Object *o2 = NULL;
    int len = 0;
    if(-1 == (fid = open (filename, O_RDONLY)))
    {
        return NULL;
    }
    close (fid);
    pathComps->num = getTokens (filename, &path, PATH_SEP_STR);
    pathComps->data = (void*)path;
    for(i = 0; i<pathComps->num; i++)
    {
        o1 = calloc (sizeof(FS_Object), 1);
        o1->parent = o2;
        o1->baseName = ((char**)pathComps->data)[i];
        o1->depth = 1;
        if(o2 && i!=pathComps->num-1)
        {
            len  = strlen (o2->baseName);
            len += (o2->dirName?strlen (o2->dirName):0);
            o1->dirName = calloc (len+2, sizeof(char));
            if(o2->dirName)
                sprintf (o1->dirName, "%s%c%s", o2->dirName, PATH_SEP_CHR,
                         o2->baseName);
            else
                sprintf (o1->dirName, "%s", o2->baseName);

            o1->depth += o2->depth;
        }
        o2 = o1;
        printer (NULL, o1, NULL);
    }
    return o1;
}

static void *
_getAllFSTokens(
    FS_Object *fs,
    char *sep,
    array *pImpFlds
    )
{
    pathFields	 *dds	      = NULL;	/* dds = dot dot slash or parent */
    pathFields	**ds	      = NULL;	/* ds = dot slash or curent */
    int		  nFldDds     = 0;	/* number of fields till dds */
    int		  nKeyFldDds  = 0;	/* number of key fields till dds */
    int		  nEntFlds    = 0;	/* number of Flds in this entry */
    char	**pEntFlds    = NULL;	/* actual fields in this entry */
    fields_t     *pKeyFlds    = NULL;	/* actual array of key fields till dds */
    int		 *impFlds     = (int*)(pImpFlds?pImpFlds->data:NULL);
    int		  nImpFlds    = (int)(pImpFlds?pImpFlds->num:0);
    int		  nNewKeyFlds = 0;
    fields_t	 *tmp	      = NULL;
    char         *cTmp        = NULL;
    int i;
    int j;

    /* get details of parent fields */
    if(fs->parent && fs->parent->userData && *(fs->parent->userData))
    {
	dds     = (pathFields*)(*(fs->parent->userData));
	nFldDds = dds->numFields;
	nKeyFldDds = dds->impFields.num;
	pKeyFlds   = (fields_t*)(dds->impFields.data);
    }

    nEntFlds = getTokens (fs->baseName, &pEntFlds, sep);
    nNewKeyFlds = nKeyFldDds;
    cTmp = getFullName (fs);
    printf ("%s:%d %d %d %s\n", __FUNCTION__, __LINE__, nEntFlds, nNewKeyFlds,
            cTmp);
    CHK_FREE (cTmp);
    /* get how many fields to store */
    for(i=0;i<nEntFlds;i++)
    {
        printf ("\t\t%d %s\n", i, pEntFlds[i]);
        if(nImpFlds == 0)
            nNewKeyFlds++;
    	for(j=0;j<nImpFlds;j++)
	    if(impFlds[j] == i+nFldDds)
		nNewKeyFlds++;
    }

    printf ("%s:%d dds=%d ent = %d new_key = %d fld_dds=%d key_fld_dds=%d \n",
            __FUNCTION__, __LINE__, nFldDds, nEntFlds, nNewKeyFlds, nFldDds, nKeyFldDds);

    ds = (pathFields**)calloc (sizeof(pathFields*), 1);
    *ds  = (pathFields*)calloc (sizeof(pathFields), 1);
    (*ds)->numFields = nFldDds + nEntFlds;
    (*ds)->impFields.num   = nNewKeyFlds;
    (*ds)->impFields.size  = sizeof(fields_t);
    if(nNewKeyFlds)
    {
        tmp = (fields_t*)calloc (sizeof(fields_t), nNewKeyFlds);
        (*ds)->impFields.data = (void*)tmp;
        for(i=0; i<nKeyFldDds; i++)
        {
            tmp[i].idx   = pKeyFlds[i].idx;
            tmp[i].field = calloc(1+strlen(pKeyFlds[i].field), sizeof(char));
            memcpy (tmp[i].field, pKeyFlds[i].field, 1+strlen(pKeyFlds[i].field));
        }
        for( j=0;i<nNewKeyFlds && j<nEntFlds;j++)
        {
            int k;            
            /* check if this is to be stored */
            for(k=0;k<nImpFlds;k++)
                if(impFlds[k] == j+nFldDds)
                    break;
            if( nImpFlds && k >= nImpFlds ) continue;
            else if ( nImpFlds == 0) k = i;
            else k = impFlds[k];
            
            printf ("%s:%d %d %d %s\n", __FUNCTION__, __LINE__, i, j, pEntFlds[j]);
            tmp[i].idx   = k;
            tmp[i].field = calloc(1+strlen(pEntFlds[j]), sizeof(char));
            memcpy (tmp[i].field, pEntFlds[j], 1+strlen(pEntFlds[j]));
            i++;
        }
    }
    for(i=0;i<nEntFlds;i++)
	CHK_FREE (pEntFlds[i]);
    CHK_FREE (pEntFlds);
    printf ("%s:%d dds=%d ent = %d new_key = %d fld_dds=%d key_fld_dds=%d \n",
            __FUNCTION__, __LINE__, nFldDds, nEntFlds, nNewKeyFlds, nFldDds, nKeyFldDds);
    return ((void*)ds);
}

int
getAllFSTokens(
    FS_Object *fs,
    void *vargs
    )
{
    char **args  = (char**)vargs;
    char  *sep   = args[0];
    array *pImpFlds = (array*)args[1];
    /* check args for patterns and anti-patterns */
    fs->userData = _getAllFSTokens(fs, sep, pImpFlds);

    printUserData (fs->userData);
    return 1;
}

#if 0
static int
_getFileTokens(
    FS_Object *fs,
    char *sep,
    array *pImpFlds
    )
{
    pathFields	 *dds	      = NULL;	/* dds = dot dot slash or parent */
    pathFields	**ds	      = NULL;	/* ds = dot slash or curent */
    int		  nFldDds     = 0;	/* number of fields till dds */
    int		  nKeyFldDds  = 0;	/* number of key fields till dds */
    int		  nEntFlds    = 0;	/* number of Flds in this entry */
    char	**pEntFlds    = NULL;	/* actual fields in this entry */
    fields_t     *pKeyFlds    = NULL;	/* actual array of key fields till dds */
    int		 *impFlds     = (int*)(pImpFlds?pImpFlds->data:NULL);
    int		  nImpFlds    = (int)(pImpFlds?pImpFlds->num:0);
    int		  nNewKeyFlds = 0;
    fields_t	 *tmp	      = NULL;
    char         *cTmp        = NULL;
    int i;
    int j;

    /* get details of parent fields */
    if(fs->parent && fs->parent->userData && *(fs->parent->userData))
    {
	dds     = (pathFields*)(*(fs->parent->userData));
	nFldDds = dds->numFields;
	nKeyFldDds = dds->impFields.num;
	pKeyFlds   = (fields_t*)(dds->impFields.data);
    }
    else if(fs->parent)
    {
        /* the parent dieectory has not been parsed for tokens, do it now */
        int numSep = strlen (sep);
        char *sep_l = calloc (numSep+2, 1);
        memcpy (sep_l, sep, numSep);
        sep_l[numSep+1] = PATH_SEP_CHR;
        nFldDds = getTokens (fs->parent->baseName, &pKeyFlds, sep);
    }

    nEntFlds = getTokens (fs->baseName, &pEntFlds, sep);
    nNewKeyFlds = nKeyFldDds;
    cTmp = getFullName (fs);
    printf ("%s:%d %d %d %s\n", __FUNCTION__, __LINE__, nEntFlds, nNewKeyFlds,
            cTmp);
    CHK_FREE (cTmp);
    /* get how many fields to store */
    for(i=0;i<nEntFlds;i++)
    {
        printf ("\t\t%d %s\n", i, pEntFlds[i]);
        if(nImpFlds == 0)
            nNewKeyFlds++;
    	for(j=0;j<nImpFlds;j++)
	    if(impFlds[j] == i+nFldDds)
		nNewKeyFlds++;
    }

    printf ("%s:%d dds=%d ent = %d new_key = %d fld_dds=%d key_fld_dds=%d \n",
            __FUNCTION__, __LINE__, nFldDds, nEntFlds, nNewKeyFlds, nFldDds, nKeyFldDds);

    ds = (pathFields**)calloc (sizeof(pathFields*), 1);
    *ds  = (pathFields*)calloc (sizeof(pathFields), 1);
    (*ds)->numFields = nFldDds + nEntFlds;
    (*ds)->impFields.num   = nNewKeyFlds;
    (*ds)->impFields.size  = sizeof(fields_t);
    if(nNewKeyFlds)
    {
        tmp = (fields_t*)calloc (sizeof(fields_t), nNewKeyFlds);
        (*ds)->impFields.data = (void*)tmp;
        for(i=0; i<nKeyFldDds; i++)
        {
            tmp[i].idx   = pKeyFlds[i].idx;
            tmp[i].field = calloc(1+strlen(pKeyFlds[i].field), sizeof(char));
            memcpy (tmp[i].field, pKeyFlds[i].field, 1+strlen(pKeyFlds[i].field));
        }
        for( j=0;i<nNewKeyFlds && j<nEntFlds;j++)
        {
            int k;            
            /* check if this is to be stored */
            for(k=0;k<nImpFlds;k++)
                if(impFlds[k] == j+nFldDds)
                    break;
            if( nImpFlds && k >= nImpFlds ) continue;
            else if ( nImpFlds == 0) k = i;
            else k = impFlds[k];
            
//            printf ("%s:%d %d %d %s\n", __FUNCTION__, __LINE__, i, j, pEntFlds[j]);
            tmp[i].idx   = k;
            tmp[i].field = calloc(1+strlen(pEntFlds[j]), sizeof(char));
            memcpy (tmp[i].field, pEntFlds[j], 1+strlen(pEntFlds[j]));
            i++;
        }
    }
    for(i=0;i<nEntFlds;i++)
	CHK_FREE (pEntFlds[i]);
    CHK_FREE (pEntFlds);
    printf ("%s:%d dds=%d ent = %d new_key = %d fld_dds=%d key_fld_dds=%d \n",
            __FUNCTION__, __LINE__, nFldDds, nEntFlds, nNewKeyFlds, nFldDds, nKeyFldDds);
    return ((void*)ds);
}

int
getFileTokens(
    FS_Object *fs,
    void *vargs
    )
{
    char **args  = (char**)vargs;
    char  *sep   = args[0];
    array *pImpFlds = (array*)args[1];
    /* check args for patterns and anti-patterns */
    fs->userData = _getFileTokens(fs, sep, pImpFlds);

    printUserData (fs->userData);
    return 1;
}
#endif

#if 1//def EIV_DBG
#define EIV_DBG_PRINTF printf
#else
#define EIV_DBG_PRINTF(...)
#endif

//void *gSortStruct = NULL;

/** 
 * 
 * 
 * @param Obj1      first file
 * @param Obj2      second file
 * @param tokens    list of tokens by whichto sort/compare
 * @param tokOrd    type of ordering of tokens (1: asc, -1 desc)x(1: char, 2:numeric)
 * @param nTokens   
 * 
 * @return          (obj1<obj2)?-1:(obj1>obj2)?1:0
 */

int
compareByToks (void *Obj1, void *Obj2, void *args)
{
    if(Obj1 && !Obj2) return -1;
    if(Obj2 && !Obj1) return 1;
    if(*(char*)Obj1 && !(*(char*)Obj2)) return -1;
    if(!(*(char*)Obj1) && *(char*)Obj2) return 1;
        
    FS_Object  *o1 = *((FS_Object**)Obj1);
    FS_Object  *o2 = *((FS_Object**)Obj2);

    pathFields *p1 = *((pathFields**)(o1->userData));
    pathFields *p2 = *((pathFields**)(o2->userData));

    fields_t *f1 = NULL;
    fields_t *f2 = NULL;

    int **sortStruct = (int**)args;//gSortStruct;
    int *tokens = sortStruct?sortStruct[0]:NULL;
    int *tokOrd = sortStruct?sortStruct[1]:NULL;
    int nTokens = *(sortStruct[2]);
    int i;
    int r = 0;

    GET_FIELDS_PTR_FSO (o1, f1);
    GET_FIELDS_PTR_FSO (o2, f2);

    EIV_DBG_PRINTF ("compare %s %s\n", getFullName (o1), getFullName (o2));
    
    for(i = 0; i< nTokens; i++)
    {
        int t = tokens?tokens[i]:i;
        int s;
        int k;
        for(k = 0; k<nTokens; k++)
        {            
            if(k<p1->impFields.num && f1[k].idx == t) break;
        }
        if ( k >= nTokens )
        {            
            EIV_DBG_PRINTF ("bad token i: %d k: %d p1->impFields: %d p2->impFields: %d\n", i, k,
                            p1->impFields.num, p2->impFields.num);
            return r;
        }
        s = tokOrd && tokOrd[k]<0?-1:1;
        t = tokOrd?tokOrd[k]/s:1;
        if(k< p1->impFields.num && k<p2->impFields.num)
        {
            if( t == 1)
                r = strcmp (f1[k].field, f2[k].field);
            else
                r = atoi (f1[k].field)-atoi (f2[k].field);
            EIV_DBG_PRINTF ("\tgood: %d %s %s %d %d\n", k, f1[k].field, f2[k].field, r, s);
        }
        else if (k>=p1->impFields.num && k < p2->impFields.num)
        {
            EIV_DBG_PRINTF ("\tp1 bad: %d %s %s %d\n", k, "NULL", f2[k].field, r);
            r=-s;
        }
        else if (k<p1->impFields.num && k >= p2->impFields.num)
        {
            EIV_DBG_PRINTF ("\tp2 bad %d %s %s %d\n", k, f1[k].field, "NULL", r);
            r= s;
        }
        else
        {
            /* worst case - fall back to comparing names */
            char *n1 = getFullName (o1);
            char *n2 = getFullName (o2);
            r = strcmp (n1, n2);
            CHK_FREE (n1);
            CHK_FREE (n2);
            
        }
        if(r) return (s*r);
    }
    return 0;
}

void sortByTokens (
    FS_Object **pObjs,
    int nObjs,
    int *sortFlds,
    int *sortOrd,
    int nSortFlds
    )
{
    int i;
    int j;
    void *sortStruct[] = {(void*)sortFlds, (void*)sortOrd, (void*)&nSortFlds};
//    gSortStruct = sortStruct;
    nObjs = 5;
    if(nObjs <= 0 || !pObjs) return ;
    EIV_DBG_PRINTF ("%p %p %p ", pObjs, pObjs[0], pObjs[1]);
    EIV_DBG_PRINTF ("%p %p %p | ", pObjs[2], pObjs[3], pObjs[4]);
    EIV_DBG_PRINTF ("%p %p ", pObjs[0]->userData, pObjs[1]->userData);
    EIV_DBG_PRINTF ("%p %p %p ", pObjs[2]->userData, pObjs[3]->userData, pObjs[4]->userData);
    EIV_DBG_PRINTF ("%s %s ", getFullName (pObjs[0]), getFullName (pObjs[1]));
    EIV_DBG_PRINTF ("%s %s ", getFullName (pObjs[2]), getFullName (pObjs[3]));
    EIV_DBG_PRINTF ("%s\n", getFullName (pObjs[4]));
#if 1
    for(i=0; i<nObjs; i++)
    {
        for(j=i+1; j<nObjs; j++)
        {
            int result = compareByToks (&pObjs[i], &pObjs[j], sortStruct);
            if( result == -1)
            {
#if 0
                SWAP (pObjs[i], pObjs[j], sizeof(FS_Object*));
#elif 0//TEST_CODE
                char *t1 = calloc (sizeof(FS_Object*), 1);
                memcpy (t1, &pObjs[i], sizeof(FS_Object*));
                memcpy (&pObjs[i], &pObjs[j], sizeof(FS_Object*));
                memcpy (&pObjs[j], t1, sizeof(FS_Object*));
#else
                FS_Object *t = pObjs[i];
                pObjs[i] = pObjs[j];
                pObjs[j] = t;
#endif
            }
        }
    }
#endif

//    qsort_r(&pObjs[0], nObjs, sizeof(FS_Object*), compareByToks, (void*)sortStruct);
}

int getIndexOfField (
    FS_Object *o1,
    int fieldIdx
    )
{
    pathFields *p1 = *(pathFields**)(o1->userData);
    fields_t *f1 = NULL;
    int i;
    GET_FIELDS_PTR_FSO (o1, f1);
    for(i=0; i<p1->impFields.num; i++)
    {
        if(fieldIdx == f1[i].idx) return i;
    }
    return -1;
}

void freeFields (FS_Object *o)
{
    fields_t *f = NULL;
    pathFields *p = NULL;
    int i;
    GET_FIELDS_PTR_FSO (o, f);
    if(f)
    {
        p = *((pathFields**)(o->userData));
        for( i = 0; i<p->impFields.num; i++)
        {
//            printf ("%d %p %s\n", i, f[i].field, f[i].field);
            CHK_FREE (f[i].field);
        }
    }
    if(p)
        CHK_FREE (p->impFields.data);
}

void freeFS_Objects (FS_Object **args, int nObjs)
{
    int i;
    FS_Object **objs = args;
    for(i=0;i<nObjs; i++)
    {
        freeFields (objs[i]);
        CHK_FREE (objs[i]->baseName);
        CHK_FREE (objs[i]->dirName);        
        CHK_FREE (*(objs[i]->userData));
        CHK_FREE (objs[i]->userData);
        CHK_FREE (objs[i]);
    }
    CHK_FREE (objs);
}
