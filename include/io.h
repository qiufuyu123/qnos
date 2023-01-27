#ifndef _H_IO
#define _H_IO
#include"types.h"
static inline void
    insl(uint32_t port, void *addr, int cnt) {
        asm volatile (
            "cld;"
            "repne; insl;"
            : "=D" (addr), "=c" (cnt)
            : "d" (port), "0" (addr), "1" (cnt)
            : "memory", "cc");
    }
void store_eflags(int flg);
int load_eflags();
static inline void insw(uint16_t port, void *addr, int cnt)
{
  asm volatile("cld; rep insw" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}
static inline void outsw(uint16_t port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsw" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
static inline void outw(uint16_t port, uint16_t val)
{
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}
static inline void outl(uint16_t port, uint32_t val)
{
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}
static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}
static inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void sti()
{
    asm volatile("sti");
}
static inline void cli()
{
    asm volatile("cli");
}

#define ICW1_BIT4	0x10	/* must be set */
#define ICW1_LTIM	0x08	/* level triggered mode (0 for edge) */
#define ICW1_ADI	0x04	/* call address interval:4 (0 for 8) */
#define ICW1_SNGL	0x02	/* sigle (0 for cascade mode)*/
#define ICW1_IC4	0x01	/* IC4 is needed */

#define ICW1		(ICW1_BIT4 | ICW1_IC4)	/* edge cascade 8interval IC4 */
#define ICW3_MASTER	0x04	/* slave PIC is cascaded on IRQ2 */
#define ICW3_SLAVE	0x02	/* ?? */

#define ICW4_uPM	0x01	/* 8086/8088 mode(0 for MCS-80/85 mode) */
#define ICW4_AEOI	0x02	/* auto EOI (0 for normal EOI) */
#define ICW4_MS		0x04	/* X non buffer|0 buffer slave|1 buffer master */
#define ICW4_BUF	0x08	/* 0 non buffer|1 buffer slave|1 buffer master */
#define ICW4_SFNM	0x10	/* special fully nested mode(non nested) */

#define ICW4_MASTER	(ICW4_uPM)	/* non-buffer 8086 NEOI non-nested */
#define ICW4_SLAVE	(ICW4_uPM)	/* non-buffer 8086 NEOI non-nested */

#define REG_PIC1	0x20
#define REG_PIC2	0xa0
#define PIC_A0		0x01	/* A0 bit */
#define PIC_MASTER_CMD	REG_PIC1		/* master command register */
#define PIC_MASTER_DATA	(REG_PIC1 | PIC_A0)	/* master data register */
#define PIC_SLAVE_CMD	REG_PIC2		/* slave command register */
#define PIC_SLAVE_DATA	(REG_PIC2 | PIC_A0)	/* slave data register */
#define PIC_MASTER_IMR	PIC_MASTER_DATA		/* interrupt mask register */
#define PIC_SLAVE_IMR	PIC_SLAVE_DATA		/* interrupt mask register */

#endif