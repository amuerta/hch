#ifndef __LINK_H
#define __LINK_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

//
// Link appendage (Linked List)
//
#ifndef li_append

#define li_append(L, I) do {\
    if(!(L)) {(L) = (I); (L)->tail = (L);}\
    else {\
        void* item = (I);\
        (L)->tail->next = item;\
        (L)->tail = item;\
    }\
}while(0)

#define li_foreach(LI, T, I, ...) do {\
   T* next = (LI);\
   T* prev = (LI); (void)prev; (void)next;\
   while(next) {\
       T* I = next;\
       {__VA_ARGS__}\
       prev = next;\
       next = next->next;\
   }\
} while(0)

#define li_defer(LI, T, ...) do {\
   T* next = (LI);\
   T* prev = (LI);\
   while(next) {\
       prev = next;\
       next = next->next;\
       {__VA_ARGS__}\
       prev = 0;\
   } (LI) = 0;\
} while(0)

#endif//li_append
#endif//__LINK_H
