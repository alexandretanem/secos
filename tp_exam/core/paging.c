#include <paging.h>
#include <tasks.h>
#include <debug.h>
#include <pagemem.h>
#include <cr.h>


__attribute__ ((aligned(4096))) pde32_t PGD_KRN[1024] ;
__attribute__ ((aligned(4096))) pte32_t PTB_KRN[1024] ;
//__attribute__ ((aligned(4096))) pte32_t PTB_KRN2[1024] ; // debug only

	

// Init kernel pages
// From 0x300.000 to 0x400.000 => 0x100.000 size 
// One page table map 1024*0x1000 = 0x400000 => Perfect, we will map the first 0x400000 bytes
void init_kernel_pages(){

	memset(PGD_KRN, 0, PAGE_SIZE);
	set_cr3(&PGD_KRN);
	

	pg_set_entry(&PGD_KRN[0], PG_KRN|PG_RW, page_nr(PTB_KRN));
	for(int i = 0 ; i < 1024 ; i++)
		pg_set_entry(&PTB_KRN[i], PG_KRN|PG_RW, i);

	// Mapping next 0x400000 ===> to 0x800000 for debugging only
	//pg_set_entry(&PGD_KRN[1], PG_KRN|PG_RW, page_nr(PTB_KRN2));
	//for(int i = 0 ; i < 1024 ; i++)
	//	pg_set_entry(&PTB_KRN2[i], PG_KRN|PG_RW, i + 1024);

}


// Init user pages
// Map the first 0x400000 bytes as kernel pages in tasks

void init_user_pages(){
	for(uint32_t i = 0 ; i < TASKS.n ; i++){
		task_t *t = &TASKS.list[i] ;

		memset(t->pgd, 0, PAGE_SIZE);
		memset(t->pdts, 0, PAGE_SIZE*MAX_PDT_PER_TASK);

		//Mapping kernel : 0 to 0x400000 : first entry in pgd
		pde32_t *pgd = t->pgd;
		pte32_t *pdt = &(t->pdts[t->nb_pdt_used*1024]);

		pg_set_entry(&pgd[0], PG_KRN|PG_RW, page_nr(pdt));
		for(int i = 0 ; i < 1024 ; i++)
			pg_set_entry(&pdt[i], PG_KRN|PG_RW, i);

		t->nb_pdt_used++;
	}
}


// Identity map one page of a task t, with the adrress phyaddr
int map_identity(task_t *t, uint32_t *phyaddr, int type){
	return map_at(t, phyaddr, phyaddr, type);
}

// Map one page : virtaddr -> phyaddr
int map_at(task_t *t, uint32_t *phyaddr, uint32_t *virtaddr, int type){

	int pgd_idx = pd32_idx(virtaddr);
	int pdt_idx = pt32_idx(virtaddr);

	debug("MAPPING [task : %d] : %p -> %p | PGD[%d] PDT[%d] \n", t->pid, virtaddr, phyaddr, pgd_idx, pdt_idx);
	
	pde32_t *pgd = &t->pgd[pgd_idx] ;
	
	if(pgd->p == 0){ // Page not already mapped
		if(t->nb_pdt_used >= MAX_PDT_PER_TASK){
			panic("Mapping error (max)");
			return 2 ;
		}

		pte32_t *pdt = &(t->pdts[t->nb_pdt_used*1024]) ;
		pg_set_entry(pgd, PG_USR|PG_RW, page_nr(pdt));
		pg_set_entry(&pdt[pdt_idx], type|PG_RW, page_nr(phyaddr));
		t->nb_pdt_used++ ;
	
	}else{ // Page already mapped in pgd 
		pte32_t *pdt = (pte32_t *) page_addr(pgd->addr) ;

		if(pdt[pdt_idx].p == 0){ //mapped in pdt ? 
			pg_set_entry(&pdt[pdt_idx], type|PG_RW, page_nr(phyaddr));

		}else{ // Page already mapped in pdt .. error
			panic("Mapping error (already)");
			return 1 ;
		}	
	}

	return 0 ;
}

void start_paging(){
	cr0_reg_t cr0 = {.raw = get_cr0()};
	cr0.pg = 1 ;
	set_cr0(cr0);
}

void print_cr3(){
	cr3_reg_t cr3 = {.raw = get_cr3()} ;
	debug("CR3 : %p\n", cr3);
}
