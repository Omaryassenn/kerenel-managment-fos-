#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
		initialize_dyn_block_system();
		cprintf("DYNAMIC BLOCK SYSTEM IS INITIALIZED\n");
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//=================================
void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//panic("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	//[2] Dynamically allocate the array of MemBlockNodes at VA USER_DYN_BLKS_ARRAY
	//	  (remember to set MAX_MEM_BLOCK_CNT with the chosen size of the array)
	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	//[4] Insert a new MemBlock with the heap size into the FreeMemBlocksList


	LIST_INIT(&(FreeMemBlocksList));
	LIST_INIT(&(AllocMemBlocksList));


	MemBlockNodes=(struct MemBlock *)USER_DYN_BLKS_ARRAY;
	MAX_MEM_BLOCK_CNT= NUM_OF_UHEAP_PAGES ;

	uint32 sizeOfMbn=MAX_MEM_BLOCK_CNT * sizeof(*MemBlockNodes);
	int sizeOFalloc=ROUNDUP(sizeOfMbn,PAGE_SIZE);
	uint32 startsva = USER_HEAP_START;
	uint32 userHeapSize = USER_HEAP_MAX - USER_HEAP_START ;
	uint32 startsvaOfalloc= USER_DYN_BLKS_ARRAY;

	sys_allocate_chunk(startsvaOfalloc  , sizeOFalloc , PERM_USER | PERM_WRITEABLE);

		initialize_MemBlocksList( MAX_MEM_BLOCK_CNT);
		struct MemBlock *MEMblock=AvailableMemBlocksList.lh_first;
		LIST_REMOVE(&(AvailableMemBlocksList),AvailableMemBlocksList.lh_first);


			 MEMblock->sva= startsva ;
			 MEMblock->size=userHeapSize;
			 LIST_INSERT_HEAD(&(FreeMemBlocksList),MEMblock);



}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

void* malloc(uint32 size)
{

	//panic("malloc() is not implemented yet...!!");

	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//==============================================================

	uint32 sizeUP = ROUNDUP(size , PAGE_SIZE);
	struct MemBlock * BLOCKTOALLOC=NULL;

	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
		BLOCKTOALLOC = alloc_block_FF(sizeUP);

		if (BLOCKTOALLOC == NULL)
		{
			return NULL;

		}
		else
		{
			insert_sorted_allocList(BLOCKTOALLOC);
			return (uint32 *)BLOCKTOALLOC->sva;

		}
	}
		return NULL;

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	FROM main memory AND free pages from page file then switch back to the user again.
//
//	We can use sys_free_user_mem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls free_user_mem() in
//		"kern/mem/chunk_operations.c", then switch back to the user mode here
//	the free_user_mem function is empty, make sure to implement it.
void free(void* virtual_address)
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] free
	// your code is here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	//akhod elblock mn al alloc  b find block w a7to f al free w ams7 mn alloc

	virtual_address=(void*)ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
	struct MemBlock * B=find_block(&AllocMemBlocksList,(uint32)virtual_address);
	//cprintf("size= %d \n" ,B->size);
	//cprintf("size= %x \n" ,B->sva);
	if(B != NULL)
		{
				uint32 size =B->size;
				uint32 sva = B->sva;
				//cprintf("size= %d \n" ,size);
				//cprintf("size= %x \n" ,sva);
				LIST_REMOVE(&AllocMemBlocksList,B);
				insert_sorted_with_merge_freeList(B);
				//LIST_INSERT_TAIL(&FreeMemBlocksList,B);
				sys_free_user_mem(sva,size);
				///cprintf("size= %d \n" ,size);
				//cprintf("size= %x \n" ,sva);

		}
	else
		return;
	//you should get the size of the given allocation using its address
	//you need to call sys_free_user_mem()
	//refer to the project presentation and documentation for details
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
	{
		//hadeel
		//DON'T CHANGE THIS CODE========================================
	        InitializeUHeap();
	        if (size == 0) return NULL ;
	        //==============================================================
		// Write your code here, remove the panic and write your code
		//panic("smalloc() is not implemented yet...!!");
		uint32 sizeUP = ROUNDUP(size , PAGE_SIZE);
			    struct MemBlock * BLOCKTOALLOC=NULL;
			  int x=  sys_isUHeapPlacementStrategyFIRSTFIT();
			    if(x==1)
			    {
			BLOCKTOALLOC=alloc_block_FF(sizeUP);
			  if(BLOCKTOALLOC!=NULL)
			  {
				  int c= sys_createSharedObject(sharedVarName, sizeUP,  isWritable,(void*)BLOCKTOALLOC->sva );
					if(c<=-1)
					{
						return NULL;
					}
					else
					{
							insert_sorted_allocList(BLOCKTOALLOC);
							return (uint32 *)BLOCKTOALLOC->sva;
					}
			}
			else
			{
				  return NULL;
			}

	}
	return (void*)NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	uint32 sizeOfsharedObject = sys_getSizeOfSharedObject( ownerEnvID ,sharedVarName );

	if  (sizeOfsharedObject == E_SHARED_MEM_NOT_EXISTS)
	{
		return NULL;
	}
	else
	{
		uint32 sizeUP = ROUNDUP(sizeOfsharedObject , PAGE_SIZE);
			struct MemBlock * BLOCKTOALLOC=NULL;
		if(sys_isUHeapPlacementStrategyFIRSTFIT())
			{
				BLOCKTOALLOC = alloc_block_FF(sizeUP);

				if (BLOCKTOALLOC != NULL)
				{
					int retID =sys_getSharedObject(ownerEnvID , sharedVarName , (uint32 *)BLOCKTOALLOC ->sva);
					if (retID < 0 )
					{
						return NULL;
					}
					else
					{
						 return (uint32 *)BLOCKTOALLOC->sva;
				}
}

}


}
	return NULL ;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// [USER HEAP - USER SIDE] realloc
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT MS3 - BONUS] [SHARING - USER SIDE] sfree()

	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}




//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//
void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");
}
