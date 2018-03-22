#include <array.h>
#include <strHelper.h>

typedef struct _test_
{
    int i;
    char c;
}test;

array* createArray (
    int num,
    int size,
    char *data)
{
    array *A = (array*)calloc (1, sizeof(array));
    A->num = num;
    A->size = size;
    A->data = (char*)calloc (size, num);
    if(data)
        memcpy (A->data, data, size*num);
    return A;
}

void freeArray (array **A)
{
    if(A && *A)
    {
        CHK_FREE ((*A)->data);
        CHK_FREE (*A);
        *A = NULL;
    }
}

char* getDataAt (array *A, int l)
{
    if(A)
        return (A->data+l*A->size);
}

void setDataAt(array *A,int l, char *p)
{
    if(A)
    {
        memcpy(A->data+l*A->size, p, A->size);
    }
}

void addElement (
    array *A,
    char *e
    )
{
    if(A)
    {
        A->num++;
        A->data = (char*)realloc (A->data, A->size*(A->num+1));
        setDataAt (A, A->num, e);
    }
}

void insertAt (
    array *A,
    int l,
    char *e    
    )
{
    if(A)
    {
        int size = A->size;
        int num = A->num;
        if(l>num)
            return;
        A->data = realloc (A->data, size*(num+1));
        memmove (A->data+(l+1)*size, A->data+l*size, size*(num-l));
        setDataAt (A, l, e);
    }
}

array * concatArrays (
    array a1,
    array a2
    )
{
    array *p = NULL;
    int i;

    if(a1.num != a2.num || a1.size != a2.size) return p;

    p = (array*)calloc (sizeof(array), 1);
    p->num = a1.num+a2.num;
    p->size = a1.size;
    p->data = calloc (p->num, p->size);
    for(i = 0; i<a1.num; i++)
    {
        setDataAt (p, i, a1.data+a1.size*i);
    }

    for(i = 0; i<a2.num; i++)
    {
        setDataAt (p, a1.num+i, a2.data+a2.size*i);
    }

    return p;
}

#if 0
int testArray(int num)
{
    array *C = createCharArray (num);
    array *I = createIntArray (num);
    array *S = createShortArray (num);
    array *_t = createStructArray (num, test);

    for(int ii = 'a'; ii<'a'+num; ii++)
    {
        test t = {ii-'a'+'f', ii-'a'+'F'};
        setCharAt (C, ii-'a', ii);
        setIntAt (I, ii-'a', ii-'a'+'A');
        setShortAt (S, ii-'a', 'Z'-(ii-'a'));
        setStructAt (_t, ii-'a', t);
    }

    for(int ii = 'a'; ii<'a'+num; ii++)
    {
        test t1;
        char c1;
        int i1;
        short s1;
        
        getCharAt (C, ii-'a', c1);
        getIntAt (I, ii-'a', i1);
        getShortAt (S, ii-'a', s1);
        getStructAt (_t, ii-'a', t1);
        printf ("%c %c %c %c %c\n", c1, i1, s1, t1.i, t1.c);
    }
    freeArray (&C);
    freeArray (&I);
    freeArray (&S);
    freeArray (&_t);
}

int main()
{
    testArray (20);
}
#endif

