#include <segmem.h>
#include <debug.h>

#define ALIGN "%15"

seg_desc_t desc[6] ;
tss_t TSS;


void print_gdt(gdt_reg_t gdtr){

    seg_desc_t *desc = gdtr.desc ;
    uint16_t count = 0 ;

    if(desc == NULL){
        debug("NULL segment descriptor");
        return  ;
    }

    debug("%20s"ALIGN"s"ALIGN"s%35s"ALIGN"s"ALIGN"s"ALIGN"s"ALIGN"s"ALIGN"s"ALIGN"s\n\n", "Raw", "Base", "Limit", "Segtype", "Ring", "Present", "Available", "Long mode", "Def. op. size", "@");

    while(count < gdtr.limit){

        debug("%20X", *desc);
        uint32_t limit = desc->limit_2 << 16 | 
                         desc->limit_1 ;

        if(desc->g == 1)
                limit = limit * 0x1000 + 0xfff ;

        uint64_t base = desc->base_3 << 24 | 
                        desc->base_2 << 16 | 
                        desc->base_1 ;

        debug(ALIGN"p", base);
        limit += base ; 
        debug(ALIGN"p", limit);

        if((desc->s & 0x1) == 1){
		char type[4] = {' ', ' ', ' ', '\0'};
                if((desc->type >> 3) && 0x1 == 1){ // Code
                        if(((desc->type >> 2) & 0x1) == 1) type[0] = 'C';  // Conforming ?
                        if(((desc->type >> 1) & 0x1) == 1) type[1] = 'R' ; // Readable ?
                        if(((desc->type >> 0) & 0x1) == 1) type[2] = 'A' ; // Accessed ? 
			debug("%32s", "[CODE]X");
                        debug("%s", type);
                }else{ // Data
                        if(((desc->type >> 2) & 0x1) == 1) type[0] = 'E' ; // Expand-down ? 
                        if(((desc->type >> 1) & 0x1) == 1) type[1] = 'W' ; // write ? 
                        if(((desc->type >> 0) & 0x1) == 1) type[2] = 'A' ; // Accessed ? 
			debug("%32s", "[DATA]R");
                        debug("%s", type);
                }
        }else{
                switch(desc->type){
                        case 0x0: debug("%35s", "Reserved [SYS]");			break;
                        case 0x1: debug("%35s", "16-bit TSS (Available) [SYS]");	break;
                        case 0x2: debug("%25s", "LDT [SYS]");				break;
                        case 0x3: debug("%25s", "16-bit TSS (Busy) [SYS]");		break;
                        case 0x4: debug("%25s", "16-bit Call Gate [SYS]");		break;
                        case 0x5: debug("%25s", "Task Gate [SYS]");			break;
                        case 0x6: debug("%25s", "16-bit Interrupt Gate [SYS]");		break;
                        case 0x7: debug("%25s", "16-bit Trap Gate [SYS]");		break;
                        case 0x8: debug("%25s", "Reserved [SYS]");			break;
                        case 0x9: debug("%35s", "32-bit TSS (Available) [SYS]");	break;
                        case 0xa: debug("%25s", "Reserved [SYS]");			break;
                        case 0xb: debug("%25s", "32-bit TSS (Busy) [SYS]");		break;
                        case 0xc: debug("%25s", "32-bit Call Gate [SYS]");		break;
                        case 0xd: debug("%25s", "Reserved [SYS]");			break;
                        case 0xe: debug("%25s", "32-bit Interrupt Gate [SYS]");		break;
                        case 0xf: debug("%25s", "32-bit Trap Gate [SYS]");		break;
                        default:  debug("%25s", "unknown [SYS]");
                }
        }

	debug(ALIGN"d", desc->dpl);
        debug(ALIGN"d", desc->p);
        debug(ALIGN"d", desc->avl);

        if((desc->s & 0x1) == 1 )
                debug(ALIGN"d", desc->l);
        else
                debug(ALIGN"s", "x");


        if(desc->d == 0)
                debug(ALIGN"s", "16-bit");
        else
                debug(ALIGN"s", "32-bit");


        debug(ALIGN"X", desc);
        debug("\n");
        desc += 1 ; //sizeof(seg_desc_t) ;
        count += sizeof(seg_desc_t);
    }

}


void set_flat_model(){

	gdt_reg_t gdtr ;
	gdtr.desc = desc ;
	gdtr.limit = sizeof(desc)-1 ;
	
	desc[0].raw = 0ULL ;

	for(int i = 1 ; i < 5 ; i++){
		// Flat 
		desc[i].base_1 = 0 ;
		desc[i].base_2 = 0 ;
		desc[i].base_3 = 0 ;
		desc[i].limit_1 = 0xffff ;
		desc[i].limit_2 = 0xf ;
		
		// Code/Data
		desc[i].s = 1 ;

		// Present
		desc[i].p = 1 ;

		// Longmode
		desc[i].l = 0 ;

		// 32 bit
		desc[i].d = 1 ;

		// Granularity
		desc[i].g = 1 ;

	}	

	// Segment types
	desc[1].type = SEG_DESC_CODE_XR ;
	desc[2].type = SEG_DESC_DATA_RW ;
	desc[3].type = SEG_DESC_CODE_XR ;
	desc[4].type = SEG_DESC_DATA_RW ;

	desc[1].dpl = 0 ;
	desc[2].dpl = 0 ;
	desc[3].dpl = 3 ;
	desc[4].dpl = 3 ;

	// TSS
	uint32_t addr = (uint32_t) &TSS ;

	desc[5].raw = 0ULL ;
	desc[5].type = SEG_DESC_SYS_TSS_AVL_32 ;
	desc[5].p = 1 ;
	desc[5].base_1 = addr ;
	desc[5].base_2 = addr >> 16 ;
	desc[5].base_3 = addr >> 24 ;
	desc[5].limit_1 = sizeof(TSS);
	desc[5].limit_2 = sizeof(TSS) >> 16;

	TSS.s0.ss = gdt_krn_seg_sel(2);
	TSS.s0.esp = get_ebp();

	set_gdtr(gdtr);
}

void set_seg_regs(){

	set_cs(gdt_krn_seg_sel(1));
	set_ss(gdt_krn_seg_sel(2));

	set_ds(gdt_krn_seg_sel(2));
	set_es(gdt_krn_seg_sel(2));
	set_fs(gdt_krn_seg_sel(2));
	set_gs(gdt_krn_seg_sel(2));

	set_tr(gdt_krn_seg_sel(5));
}

