#include <debug.h>
#include <tasks.h>
#include <intr.h>

task_t CURRENT ;
tasks_t TASKS = {};

extern void resume_from_intr() ;

// PGD and PDT for all tasks
__attribute__ ((aligned(4096))) pde32_t PGD[MAX_TASKS][1024];
__attribute__ ((aligned(4096))) pte32_t PDTS[MAX_TASKS][MAX_PDT_PER_TASK][1024];


void init_tasks(){
	TASKS.n = 0 ;
}

int new_task(uint32_t *code){

	if(TASKS.n >= sizeof(TASKS.list)/sizeof(task_t)){
		panic("Too many tasks already created ...\n");
		return 1 ;
	}

	int n = TASKS.n ;
	task_t *t = &TASKS.list[n] ;

	t->pid = n ;
	t->user_stack = (uint32_t*) (TOPUSERSTACK - USER_STACK_SIZE * n) ;
	t->kernel_stack = (uint32_t*) (TOPKERNELSTACK - KERNEL_STACK_SIZE * n) ;
	t->code = code ;

	t->pgd = PGD[n];
	t->pdts = PDTS[n][0];
	t->nb_pdt_used = 0 ;

	TASKS.n++ ;

	return 0 ;
}

void print_tasks(){
	debug("\n%20s%20s%20s%20s%20s%20s%20s%20s\n", "PID", "@ User stack", "@ Kernel stack", "@ Code", "@ ESP", "@ PGD", "@PDTS", "NB pdt used");
	for(uint32_t i = 0 ; i < TASKS.n ; i++){
		task_t t = TASKS.list[i];
		debug("%20x%20x%20x%20x%20x%20x%20x%20d\n", t.pid, t.user_stack, t.kernel_stack, t.code, t.saved_esp, t.pgd, t.pdts, t.nb_pdt_used);
	}
}


void init_stack(task_t *t){
	t->saved_esp = t->kernel_stack ;
	
	// Set context on the stack
	*(--t->saved_esp) = (uint32_t) gdt_usr_seg_sel(4) ;	// SS
	*(--t->saved_esp) = (uint32_t) t->user_stack ;		// ESP
	*(--t->saved_esp) = (uint32_t) EFLAGS_IF ;		// EFLAGS => re-enable interruptions after iret
	*(--t->saved_esp) = (uint32_t) gdt_usr_seg_sel(3) ;	// CS
	*(--t->saved_esp) = (uint32_t) t->code ;		// EIP

	*(--t->saved_esp) = (uint32_t) 0; 			// in resume_from_intr :  
	*(--t->saved_esp) = (uint32_t) 0; 			// add $8, $esp

	uint32_t temp = (uint32_t) t->saved_esp ;

	// PUSHA
	*(--t->saved_esp) = (uint32_t) 0 ; 			// EAX	
	*(--t->saved_esp) = (uint32_t) 0 ; 			// ECX	
	*(--t->saved_esp) = (uint32_t) 0 ; 			// EDX	
	*(--t->saved_esp) = (uint32_t) 0 ; 			// EBX	
	*(--t->saved_esp) = (uint32_t) temp ; 			// ESP	
	*(--t->saved_esp) = (uint32_t) t->kernel_stack - 1 ;	// EBP	
	*(--t->saved_esp) = (uint32_t) 0 ; 			// ESI	
	*(--t->saved_esp) = (uint32_t) 0 ; 			// EDI	

	*(--t->saved_esp) = (uint32_t) &resume_from_intr; 	// RET

	*(--t->saved_esp) = (uint32_t) 0 ; 			// Saved ebp popped in switch.s
								// Unuseful on first interruption => direct return to resume_from intr

}
