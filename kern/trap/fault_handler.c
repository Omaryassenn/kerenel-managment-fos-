/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"
#include <kern/disk/pagefile_manager.h>



//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}
void placement (struct Env * curenv , uint32 fault_va)
{
	//fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);
	for( uint32 i =0; i<curenv->page_WS_max_size ;i++)
	{
		if(curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].empty==1) break;
		curenv->page_last_WS_index++;
		curenv->page_last_WS_index %= (curenv->page_WS_max_size);
	}
	struct FrameInfo *frame_info_ptr = NULL;
		if ((allocate_frame(&frame_info_ptr))!=0)
			{
			return;
			}
		int s =0 ;
		s=map_frame(curenv->env_page_directory, frame_info_ptr, fault_va,  PERM_USER | PERM_WRITEABLE);
		if(s!=0)
			{
			return;
			}

		s = pf_read_env_page(curenv, (void *) fault_va);
		if (s == E_PAGE_NOT_EXIST_IN_PF)
		{
			if ((fault_va < USTACKTOP && fault_va >= USTACKBOTTOM)||(fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX))
			{
				//IT'S OKAY.
			}
			else
			{
				unmap_frame(curenv->env_page_directory, fault_va);
				panic("ILLEGAL MEMORY ACCESS");
			}
		}
		//cprintf("Before...................................\n");
		////env_page_ws_print(curenv);

		env_page_ws_set_entry(curenv,curenv->page_last_WS_index,ROUNDDOWN(fault_va,PAGE_SIZE));
		curenv->page_last_WS_index++;
		curenv->page_last_WS_index %= (curenv->page_WS_max_size);

		//cprintf("After...................................\n");
		//env_page_ws_print(curenv);
		//cprintf("va: %p \n",ROUNDDOWN(curenv->__uptr_pws[0].virtual_address,PAGE_SIZE));

}
void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//cprintf("Start Fault Handler , va: %p\n", fault_va);
	if(env_page_ws_get_size(curenv) < curenv->page_WS_max_size)
	{
		placement(curenv,fault_va);
		//cprintf("Done Placement!\n");
	}
	else
	{
		uint32 * PT=NULL;
		uint32 maxSize =curenv->page_WS_max_size;
		uint32 lastIndex=curenv->page_last_WS_index;
		for(uint32 i=lastIndex ; ;i = (i + 1)%(maxSize))
		{
			//cprintf("i = %d..................................\n", i);
			//env_page_ws_print(curenv);
			//cprintf("WS index: %d \n", i);
			get_page_table(curenv->env_page_directory,(curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address),&PT);
			if(PT == NULL)
				{return;}
			bool used =PT[PTX((curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address))]&PERM_USED;
			if(used)
			{
				//usedbit=1
				PT[PTX((curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address))] &= (~PERM_USED);
				curenv->page_last_WS_index++;
				if(curenv->page_last_WS_index==curenv->page_WS_max_size)
					curenv->page_last_WS_index %= (curenv->page_WS_max_size);
				//cprintf("Done Changing used!\n");
			}
			else
			{
				//usedbit=0
				bool m = PT[PTX((curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address))]&PERM_MODIFIED;
				if(!m)
				{
					//not modified
					unmap_frame(curenv->env_page_directory,(curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address));
					env_page_ws_clear_entry(curenv ,i);
				}
				else
				{
					//modified
					struct FrameInfo *fi=get_frame_info(curenv->env_page_directory , (curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address),&PT);
					pf_update_env_page(curenv , (curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address) , fi);
					unmap_frame(curenv->env_page_directory,(curenv->ptr_pageWorkingSet[curenv->page_last_WS_index].virtual_address));
					env_page_ws_clear_entry(curenv ,i);
				}
				placement(curenv , fault_va);
				//cprintf("After Placement..................................\n");
			///	env_page_ws_print(curenv);
				return;
			}
		}
	}
}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	// Write your code here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");
}
