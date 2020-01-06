#include <user.h>
#include <debug.h>
#include <tasks.h>

#define WAIT 600000

void user1(){
	uint32_t *shared = SHARED_VIRT_1 ;
	while(1){
		(*shared)++;

		
	}
}

void user2(){
	uint32_t *shared = SHARED_VIRT_2 ;
	
	//test_user1();

	while(1){
		for(int i = 0 ; i < WAIT ; i++); // In order to print less lines
		sys_counter(shared);
	}
}

// Passing arg (address of uint32_t shared variable) by register (eax) 
// __attribute__ ((section(".user"))) __regparm__(1) void sys_counter();
void sys_counter(){
	asm volatile("int $0x80 \t\n");
}

void test_user1(){
	// 	############
	//	##  TESTS ##
	//	############
	//	Several tests to check that user1 to other parts of memory
	//
	// TEST 1
	debug("Never printed ! \n"); // => Must generate a page fault !
	//
	// TEST 2
	// Must print nothing (not in shared page): 
	//uint32_t * test = (uint32_t *) ((uint32_t) SHARED_VIRT_2 + 0xffd) ; 	//=> print nothing
	//uint32_t * test = (uint32_t *) ((uint32_t) SHARED_VIRT_2 - 1 ) ; 	//=> print nothing
	//uint32_t * test = (uint32_t *) ((uint32_t) SHARED_VIRT_2 + 0xffc) ; 	//=> print something
	//sys_counter(test);
	//while(1);
	//
	// TEST 3
	//uint32_t *a = TASKS.list[0].code ; 
	//uint32_t *a = TASKS.list[0].user_stack - 10 ; 
	//uint32_t *a = TASKS.list[0].kernel_stack - 10 ; 
	//uint32_t *a = (uint32_t *) 0x300000 ; // Kernel stack 
	//uint32_t *a = (uint32_t *) 0x302010 ; // Kernel code
	//
	//(*a)++ ; // Must generate page fault with all a
	//
	//	#########
	//	## END ##
	//	#########

}
