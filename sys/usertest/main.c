#include"process/syscall.h"
int main()
{
    int a;
    asm volatile("int $0x80" : "=a" (a) : "0" (0), "b" ((int)4));
    return 989;
}