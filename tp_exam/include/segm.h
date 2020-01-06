#ifndef __EXAM_SEGM_H__
#define __EXAM_SEGM_H__

#include <segmem.h>

void print_gdt(gdt_reg_t gdtr);
void set_flat_model();
void set_seg_regs();
#endif
