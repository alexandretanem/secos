/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

// Files for exam in ./core and ./include
#include <segm.h>
#include <ordo.h>
#include <user.h>
#include <tasks.h>
#include <paging.h>

#include <intr.h>
#include <segmem.h>


extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;
extern uint32_t __user_start__;
extern uint32_t __user_end__;

void tp(){
	
	debug("\n\n");
	debug("██████╗██████╗  ██████╗ ███╗   ███╗██████╗  ██████╗ ███████╗	\n");
	debug("██╔════╝██╔══██╗██╔═████╗████╗ ████║██╔══██╗██╔═══██╗██╔════╝	\n");
	debug("██║     ██████╔╝██║██╔██║██╔████╔██║██║  ██║██║   ██║███████╗	\n");
	debug("██║     ██╔══██╗████╔╝██║██║╚██╔╝██║██║  ██║██║   ██║╚════██║	\n");
	debug("╚██████╗██║  ██║╚██████╔╝██║ ╚═╝ ██║██████╔╝╚██████╔╝███████║	\n");
	debug(" ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚═════╝  ╚═════╝ ╚══════╝	\n");

	debug("\n####################### \n");
	debug("#####  Memory map ##### \n");
	debug("####################### \n");
        debug("kernel code [0x%x - 0x%x]\n", &__kernel_start__, &__kernel_end__);
        debug("User code [0x%x - 0x%x]\n", &__user_start__, &__user_end__);
	debug("Kernel Stack top : 0x%x\n", TOPKERNELSTACK);
	debug("User Stack top : 0x%x\n", TOPUSERSTACK);
	debug("Shared page : 0x%x\n", SHARED_PAGE);

	
	

//////////////////////  SEGMENTATION  ///////////////////////////	
	debug("\n######################## \n");
	debug("##### Segmentation #####\n");
	debug("######################## \n");

	debug("\n#### New GDT ####\n");
	set_flat_model();
	
	gdt_reg_t gdtr ;
	get_gdtr(gdtr);
	print_gdt(gdtr);

	debug("Updating segmentation registers ...\n");
	set_seg_regs();

/////////////////////  CREATING TASKS   /////////////////////////	
	debug("\n######################## \n");
	debug("#### Creating tasks #### \n");
	debug("######################## \n");
	init_tasks();	
	new_task((uint32_t*) &user1);
	new_task((uint32_t*) &user2);

	for(unsigned int i = 0 ; i < TASKS.n ; i++){
		debug("-> init stack of task %d\n", i);
		init_stack(&TASKS.list[i]);
	}



////////////////////////  PAGINATION  //////////////////////////
	debug("\n######################### \n");
	debug("#### Init pagination #### \n");
	debug("######################### \n");
	init_kernel_pages();
	init_user_pages();

	// Page map code, user_stack, kernel_stack pages for tasks 
	for(unsigned int i = 0 ; i < TASKS.n ; i++){
		task_t *t = &TASKS.list[i] ;
		debug("[user_stack] ");		map_identity(t, (uint32_t *) (((uint32_t) t->user_stack) - PAGE_SIZE), PG_USR); 	// - PAGE_SIZE => grows down
		debug("[kernel_stack] ");	map_identity(t, (uint32_t *) (((uint32_t) t->kernel_stack) - PAGE_SIZE), PG_KRN); 	// - PAGE_SIZE => grows down
		debug("[code] ");		map_identity(t, t->code, PG_USR) ;
	}

	// Map shared page
	debug("[shared_page]"); 	map_at(&TASKS.list[0], (uint32_t *) SHARED_PAGE, SHARED_VIRT_1, PG_USR);
	debug("[shared page]");		map_at(&TASKS.list[1], (uint32_t *) SHARED_PAGE, SHARED_VIRT_2, PG_USR);

	debug("Start pagination ...");
	start_paging();
	debug(" DONE\n");


///////////////////////    GOOOOO !   /////////////////////////
		
	debug("\n#################### \n");
	debug("#### Tasks list #### \n");	
	debug("#################### \n");
	print_tasks();
	CURRENT = TASKS.list[1] ;


	debug("\n######################## \n");
	debug("#### Starting tasks #### \n");
	debug("######################## \n");
	
	init_clock_interrupt();
	
	while(1);
	debug("༼ つ ╹ ╹ ༽つ\n");
}

