#include"utils/qhash.h"
QHASH get_qhash(const char * key)
{
    uint32_t      len ;
    unsigned    hash ;
    uint32_t      i ;

    if (!key)
        return 0 ;

    len = strlen(key);
    for (hash=0, i=0 ; i<len ; i++) {
        hash += (unsigned)key[i] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }
    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);
    return hash ;
}