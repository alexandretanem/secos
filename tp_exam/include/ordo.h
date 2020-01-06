#ifndef __EXAM_ORDO_H__
#define __EXAM_ORDO_H__

#include <intr.h>

extern void resume_from_intr();
void init_clock_interrupt();
void syscall_handler(int_ctx_t *ctx);
void clock_handler(int_ctx_t *ctx);
void first_handler();

#endif
