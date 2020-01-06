#include <intr.h>
#include <ordo.h>

#include <debug.h>
#include <asm.h>
#include <tasks.h>
#include <cr.h>
#include <user.h>

extern uint32_t __kernel_start__ ;
extern uint32_t __kernel_end__ ;
extern uint32_t __user_start__ ;
extern uint32_t __user_end__ ;

extern tss_t TSS ;
	
extern void __regparm__(3) switch_task();
extern void __regparm__(2) switch_first();

void init_clock_interrupt(){
	asm volatile("sti");
}

bool_t first_interruption = true ;

void syscall_handler(int_ctx_t *ctx){
	
	//debug("[%x]\n", ctx->gpr.eax.raw);

	// The address is passed in eax
	// Allow only user2 if trying to print shared page
	if(	ctx->gpr.eax.raw  >= 	(uint32_t) SHARED_VIRT_2 && 
		ctx->gpr.eax.raw +3 < 	((uint32_t) SHARED_VIRT_2 + 0x1000) &&
		CURRENT.pid == 1 ){
		
		raw32_t *addr = (raw32_t *) ctx->gpr.eax.raw ;
		debug("%x\n", *addr);
	}
}


void clock_handler(int_ctx_t *ctx){

	// No need to save context on first interruption
	if(first_interruption){
		debug("INTERRUPTION ! EIP : %x ... Starting task n°%d\n", ctx->eip.raw, CURRENT.pid);
		first_interruption = false ;
		first_handler();
	}

	int t = CURRENT.pid ;
	uint32_t **saved_esp = &(CURRENT.saved_esp) ;
	int next = (CURRENT.pid + 1) % TASKS.n ;
	CURRENT = TASKS.list[next];

	debug("INTERRUPTION ! EIP : %x ... %d -> %d | ", ctx->eip.raw, t, CURRENT.pid);

	if(ctx->eip.raw >= (uint32_t) &__kernel_start__ && ctx->eip.raw < (uint32_t) &__kernel_end__){
		//debug("[KER]");
	}else if(ctx->eip.raw >= (uint32_t) &__user_start__ && ctx->eip.raw < (uint32_t) &__user_end__){
		//debug("[USER]");
	}else{
		panic("[UNKNOWN PLACE]\n");
	}

	debug("\n");	
	TSS.s0.esp = (uint32_t) CURRENT.kernel_stack ;
	switch_task(CURRENT.saved_esp, saved_esp, CURRENT.pgd); 
}


void first_handler(){
		TSS.s0.esp = (uint32_t) CURRENT.kernel_stack ;
		switch_first(CURRENT.saved_esp, CURRENT.pgd);
}
