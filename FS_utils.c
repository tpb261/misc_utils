
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
    pathComps->num = getTokens (filename, &path, '/');
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

static void*
_getFSTokens(
    FS_Object *fs,
    char sep,
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
    printf ("%s:%d %d %d %s\n", __FUNCTION__, __LINE__, nEntFlds, nNewKeyFlds,
            getFullName (fs));
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
    tmp = (fields_t*)calloc (sizeof(fields_t), nNewKeyFlds);
    (*ds)->impFields.data = (void*)tmp;

    memcpy (tmp, pKeyFlds, sizeof(fields_t)*nKeyFldDds);
    for( i=nKeyFldDds, j=0;i<nNewKeyFlds && j<nEntFlds;j++)
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
    for(i=0;i<nEntFlds;i++)
	CHK_FREE (pEntFlds[i]);
    printf ("%s:%d dds=%d ent = %d new_key = %d fld_dds=%d key_fld_dds=%d \n",
            __FUNCTION__, __LINE__, nFldDds, nEntFlds, nNewKeyFlds, nFldDds, nKeyFldDds);
    return ((void*)ds);
}

void *
getFSTokens(
    FS_Object *fs,
    void *vargs
    )
{
    char **args  = (char**)vargs;
    char   sep   = *(args[0]);
    array *pImpFlds = (array*)args[1];

    fs->userData = _getFSTokens(fs, sep, pImpFlds);

    printUserData (fs->userData);
    return NULL;
}
