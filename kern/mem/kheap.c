#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
/*
 * LIST_INIT(&(AllocMemBlocksList));
	LIST_INIT(&(FreeMemBlocksList));
if( STATIC_MEMBLOCK_ALLOC)
	return 0;
else
{
	 MAX_MEM_BLOCK_CNT=NUM_OF_KHEAP_PAGES;
	 uint32 sizeMBN=ROUNDUP(MAX_MEM_BLOCK_CNT,PAGE_SIZE);
	 	alloc_chunk(ptr_page_directory , KERNEL_HEAP_START , sizeMBN , PAGE_SIZE);
}

	 initialize_MemBlocksList( MAX_MEM_BLOCK_CNT);
	 struct MemBlock *MEMblock=AvailableMemBlocksList.lh_first;
	 LIST_REMOVE(&(AvailableMemBlocksList),AvailableMemBlocksList.lh_first);
	 MEMblock->sva=KERNEL_HEAP_START+sizeMBN;
	  MEMblock->size=(KERNEL_HEAP_MAX-KERNEL_HEAP_START)-sizeMBN;
	  LIST_INSERT_HEAD(&(FreeMemBlocksList),MEMblock);
 */
//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//
//int MAX_MEM_BLOCK_CNT = NUM_OF_KHEAP_PAGES;
void initialize_dyn_block_system()
{
	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	LIST_INIT(&(FreeMemBlocksList));
	LIST_INIT(&(AllocMemBlocksList));

#if STATIC_MEMBLOCK_ALLOC
	//DO NOTHING

#else
	MemBlockNodes=(struct MemBlock *)KERNEL_HEAP_START;
	MAX_MEM_BLOCK_CNT=NUM_OF_KHEAP_PAGES;
	uint32 sizeMBN=MAX_MEM_BLOCK_CNT * sizeof(*MemBlockNodes);
	int z=ROUNDUP(sizeMBN,PAGE_SIZE);
	allocate_chunk(ptr_page_directory , KERNEL_HEAP_START , z , PERM_WRITEABLE);

#endif


	initialize_MemBlocksList( MAX_MEM_BLOCK_CNT);
	struct MemBlock *MEMblock=AvailableMemBlocksList.lh_first;
	LIST_REMOVE(&(AvailableMemBlocksList),AvailableMemBlocksList.lh_first);
	 MEMblock->sva=KERNEL_HEAP_START+z;
	  MEMblock->size=(KERNEL_HEAP_MAX-KERNEL_HEAP_START)-z;
	  LIST_INSERT_HEAD(&(FreeMemBlocksList),MEMblock);

}
void* kmalloc(unsigned int size)
{
	//cprintf("size : %d \n", size);

		uint32 sizeUP = ROUNDUP(size , PAGE_SIZE);
		struct MemBlock * BLOCKTOALLOC=NULL;
		if(isKHeapPlacementStrategyFIRSTFIT())
		{
		//	cprintf("1\n");
			BLOCKTOALLOC=alloc_block_FF(sizeUP);
		//	cprintf("alloxff\n");

			if(BLOCKTOALLOC!=NULL)
			{
			//	cprintf("perm\n");
				int allocate =allocate_chunk(ptr_page_directory,BLOCKTOALLOC->sva ,sizeUP ,PERM_WRITEABLE);
				if(allocate==0)
				{
				//	cprintf("allocate");
					insert_sorted_allocList(BLOCKTOALLOC);
					//cprintf("sva: %p\n", BLOCKTOALLOC->sva);
					return (uint32 *)BLOCKTOALLOC->sva;

				}
				else
				{
					//cprintf("1/4");
					return NULL;
				}
			}
		}
		else if(isKHeapPlacementStrategyBESTFIT())
		{
			//cprintf("2");
			BLOCKTOALLOC=alloc_block_BF(sizeUP);
			if(BLOCKTOALLOC!=NULL)
			{
				int allocate =allocate_chunk(ptr_page_directory,BLOCKTOALLOC->sva ,sizeUP ,PERM_WRITEABLE);
				if(allocate==0)
				{
					insert_sorted_allocList(BLOCKTOALLOC);
					return (uint32 *)BLOCKTOALLOC->sva;
				}
				else
				{
				//	cprintf("2/4");
					return NULL;
				}
			}
		}
			else if(isKHeapPlacementStrategyNEXTFIT())
			{
				//cprintf("3");

				BLOCKTOALLOC=alloc_block_NF(sizeUP);

				if(BLOCKTOALLOC!=NULL)
				{
					int allocate =allocate_chunk(ptr_page_directory,BLOCKTOALLOC->sva ,sizeUP ,PERM_WRITEABLE);
					if(allocate==0)
					{
						insert_sorted_allocList(BLOCKTOALLOC);
						return (uint32 *)BLOCKTOALLOC->sva;
					}
				}
				else
				{
				//	cprintf("3/4");
					return NULL;
				}
			}

		return NULL;
}

		//TODO: [PROJECT MS2] [KERNEL HEAP] kmalloc
		// your code is here, remove the panic and write your code
	//	kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer

void kfree(void* virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
		struct MemBlock * block_to_free=NULL;
		block_to_free	= find_block(&AllocMemBlocksList, (uint32)virtual_address);
		//cprintf("before \n");
		if(block_to_free != NULL)
		{
			//cprintf("inside if \n");
			uint32 size =block_to_free->size;
			uint32 sva = ROUNDDOWN((uint32)virtual_address, PAGE_SIZE);
			uint32 eva = ROUNDUP((uint32)virtual_address + size, PAGE_SIZE);
			LIST_REMOVE(&AllocMemBlocksList,block_to_free);

				for (uint32 i = sva; i < eva; i += PAGE_SIZE)
				{
					//cprintf("loop \n");
					unmap_frame(ptr_page_directory, i);
				}
			insert_sorted_with_merge_freeList(block_to_free);
		}
		//cprintf("else \n");
		return;
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
	struct FrameInfo *ptr_frame_info=to_frame_info(physical_address);
	if(ptr_frame_info!=NULL)
	{
		//cprintf("%p",ptr_frame_info->va);
		return (uint32)ptr_frame_info->va;
	}
	return 0;

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	uint32 * ptr_page_table =NULL;
	struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory,virtual_address, &ptr_page_table);
	if(ptr_frame_info!=NULL)
	{
		return (uint32)to_physical_address(ptr_frame_info);
	}
	return 0;
	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT MS2 - BONUS] [KERNEL HEAP] krealloc
	// Write your code here, remove the panic and write your code
	panic("krealloc() is not implemented yet...!!");
}
