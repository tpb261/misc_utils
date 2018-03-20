#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _array_s_
{
    int num;
    int size;
    char *data;
}array;

#define createStructArray(num, s) createArray (num, sizeof(s), NULL)
#define createCharArray(num) createArray (num, sizeof(char), NULL)
#define createIntArray(num) createArray (num, sizeof(int), NULL)
#define createShortArray(num) createArray (num, sizeof(short), NULL)
#define createLongArray(num) createArray (num, sizeof(long), NULL)
#define createPtrArray(num)  createArray (num, sizeof(char*), NULL)

#define getCharAt(A, l, c) do{char *p=getDataAt(A, l); c=*p;} while(0)
#define getIntAt(A, l, i)  do{int  *p=(int*)getDataAt(A, l); i=*p;} while(0)
#define getShortAt(A, l, s) do{short *p=(short*)getDataAt(A, l); s=*p;} while(0)
#define getStructAt(A, l, s) do{memcpy (&s, getDataAt(A, l), sizeof(s));}while(0)

#define setCharAt(A, l, _) do{char c = (_); setDataAt (A, l, &c);}while(0)
#define setIntAt(A, l, _) do{int i = (_); setDataAt(A, l, &i);}while(0)
#define setShortAt(A, l, _) do{short s = (_); setDataAt(A, l, &s);}while(0)
#define setStructAt(A, l, s) setDataAt(A, l, &s)

#define insertCharAt(A, l, _)  do{char c = (_);  insertDataAt(A, l, &c);}while(0)
#define insertIntAt(A, l, _)   do{int i = (_);   insertDataAt(A, l, &i);}while(0)
#define insertShortAt(A, l, _) do{short s = (_); insertDataAt(A, l, &s);}while(0)
#define insertStructAt(A, l, s) insertDataAt(A, l, &s)

#define addChar(A, _)  do{char c = (_);  addElement(A, &c);}while(0)
#define addInt(A, _)   do{int i = (_);   addElement(A, &i);}while(0)
#define addShort(A, _) do{short s = (_); addElement(, l, &s);}while(0)
#define addStruct(A, s) addElement(A, &s)

#endif /*#ifndef _ARRAY_H_ */
