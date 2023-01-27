#ifndef _H_KTTY
#define _H_KTTY

#define TTY_LOCKED -2
enum
{
    TTY_SETXY=1,
    TTY_ATTRLOCK,
    TTY_SETFC,
    TTY_SETBC
};
#ifndef __USER__LIB

//Kernel Codes Here

#endif
#endif