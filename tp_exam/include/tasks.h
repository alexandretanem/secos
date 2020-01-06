#ifndef __EXAM_TASKS_H__
#define __EXAM_TASKS_H__

#include <types.h>
#include <pagemem.h>

// Adresses
#define USERCODE 	0x400000
#define TOPKERNELSTACK	0x600000 // Grows down
#define TOPUSERSTACK 	0x700000 // Grows down
#define SHARED_PAGE	0x800000 

#define MAX_TASKS		2 
#define USER_STACK_SIZE		0x1000
#define KERNEL_STACK_SIZE	0x1000
#define MAX_PDT_PER_TASK	5	// Kernel stack, kernel code, user stack, code, shared


typedef struct task_t {
	int pid ;

	uint32_t *user_stack ;
	uint32_t *kernel_stack ;
	uint32_t *code ;

	uint32_t *saved_esp ;

	pde32_t *pgd ;
	pte32_t *pdts ;
	int nb_pdt_used ;

} task_t ;

typedef struct tasks {
	task_t list[MAX_TASKS] ;
	uint32_t n ; 
} tasks_t ;

extern task_t CURRENT ;
extern tasks_t TASKS ;

void init_tasks();
int new_task(uint32_t *code);
void print_tasks();
void init_stack(task_t *t);

#endif
