#ifndef __EXAM_USER_H__
#define __EXAM_USER_H__

#include <types.h>

#define SHARED_VIRT_1	(uint32_t *) 0x1000000
#define SHARED_VIRT_2	(uint32_t *) 0x2000000

__attribute__ ((section(".user"))) __attribute__((aligned(4096))) void user1();
__attribute__ ((section(".user"))) __attribute__((aligned(4096))) void user2();
__attribute__ ((section(".user"))) __regparm__(1) void sys_counter();

#endif
