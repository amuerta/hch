#ifndef __LIST_H
#define __LIST_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

//
// Linked List
//
#ifndef li_append

#define li_append(L, I) do {\
    if(!(L)) {(L) = (I); (L)->tail = (L);}\
    else {\
        void* __list_item__ = (I);\
        (L)->tail->next = __list_item__;\
        (L)->tail = __list_item__;\
    }\
}while(0)

#define li_next(LI) ((LI)->next)

#define li_foreach(LI, T, I, ...) do {\
   T* __next__ = (LI);\
   T* __prev__ = (LI); (void)__prev__; (void)__next__;\
   while(__next__) {\
       T* I = __next__;\
       {__VA_ARGS__}\
       __prev__ = __next__;\
       __next__ = __next__->next;\
   }\
} while(0)

#define li_defer(LI, T, ...) do {\
   T* __next__ = (LI);\
   T* __prev__ = (LI);\
   while(__next__) {\
       __prev__ = __next__;\
       __next__ = __next__->next;\
       {__VA_ARGS__}\
       __prev__ = 0;\
   } (LI) = 0;\
} while(0)

#endif//li_append
#endif//__LINK_H
