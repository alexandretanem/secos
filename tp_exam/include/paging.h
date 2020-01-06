#ifndef __EXAM_PAGING_H__
#define __EXAM_PAGING_H__

#include <tasks.h>

void init_kernel_pages();
void init_user_pages();
void start_paging();

void print_cr3();
int map_identity(task_t *t, uint32_t *phyaddr, int type);
int map_at(task_t *t, uint32_t *phyaddr, uint32_t *virtaddr, int type);
#endif
