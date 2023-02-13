/*
 * dyn_block_management.c
 *
 *  Created on: Sep 21, 2022
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// PRINT MEM BLOCK LISTS:
//===========================

void print_mem_block_lists()
{
	cprintf("\n=========================================\n");
	struct MemBlock* blk ;
	struct MemBlock* lastBlk = NULL ;
	cprintf("\nFreeMemBlocksList:\n");
	uint8 sorted = 1 ;
	LIST_FOREACH(blk, &FreeMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nFreeMemBlocksList is NOT SORTED!!\n") ;

	lastBlk = NULL ;
	cprintf("\nAllocMemBlocksList:\n");
	sorted = 1 ;
	LIST_FOREACH(blk, &AllocMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nAllocMemBlocksList is NOT SORTED!!\n") ;
	cprintf("\n=========================================\n");

}

//********************************************************************************//
//********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//===============================
// [1] INITIALIZE AVAILABLE LIST:
//===============================
void initialize_MemBlocksList(uint32 numOfBlocks)
{


	LIST_INIT(&(AvailableMemBlocksList));

	for(int i =0 ; i<numOfBlocks ;i++)
	{
		if(i==0)
		{
			LIST_INSERT_HEAD(&(AvailableMemBlocksList),&MemBlockNodes[i]);
		}
		else
		{
			LIST_INSERT_TAIL(&(AvailableMemBlocksList),&MemBlockNodes[i]);
		}
	}

}

//===============================
// [2] FIND BLOCK:
//===============================
struct MemBlock *find_block(struct MemBlock_List *blockList, uint32 va)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] find_block
	// Write your code here, remove the panic and write your code
	//panic("find_block() is not implemented yet...!!");
	struct MemBlock * TEMP=NULL;
	struct MemBlock * TEMPx=NULL;
	LIST_FOREACH(TEMP , blockList)
	{
			if(TEMP->sva==va)
			{
				TEMPx=TEMP;
			}
	}
	return TEMPx;
}

//=========================================
// [3] INSERT BLOCK IN ALLOC LIST [SORTED]:
//=========================================
void insert_sorted_allocList(struct MemBlock *blockToInsert)
{
	struct MemBlock * curr;
	int size = LIST_SIZE(&(AllocMemBlocksList));
	if(size==0)
	{
			LIST_INSERT_HEAD(&(AllocMemBlocksList),blockToInsert);
	}
	else
	{
		curr=NULL;
		LIST_FOREACH(curr, &(AllocMemBlocksList))
		{
				struct MemBlock * prev=NULL;
				prev=curr->prev_next_info.le_prev;
				if(blockToInsert->sva < curr->sva)
				{
					LIST_INSERT_BEFORE(&(AllocMemBlocksList),curr, blockToInsert);
					return;
				}
		}
		LIST_INSERT_TAIL(&(AllocMemBlocksList),blockToInsert);

		}
	}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================

struct MemBlock *alloc_block_FF(uint32 size)
{
	struct MemBlock * temp;
	LIST_FOREACH(temp, &(FreeMemBlocksList))
	{
		struct MemBlock foundblock ;
		if(temp->size==size)
		{
			//cprintf("alloc ff if \n");
			LIST_REMOVE(&(FreeMemBlocksList),temp);
			return temp;
		}
		else if(temp->size > size)
		{
			//cprintf("alloc ff elseif \n");

			struct MemBlock* newblock=LIST_LAST(&AvailableMemBlocksList);
			//cprintf("New block : %p\n", newblock);
			newblock->sva = temp->sva;
			newblock->size =size;
			temp->sva = temp->sva + size;
			temp->size=temp->size-size;
			//cprintf("new block sva = %p \n" , newblock->sva);
			LIST_REMOVE(&(AvailableMemBlocksList),newblock);

			return newblock;
		}
	}
	return NULL;
}

//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
struct MemBlock *alloc_block_BF(uint32 size)
{
	struct MemBlock * it;
	struct MemBlock * temp=AvailableMemBlocksList.lh_last;
	uint32 min_diff=UINT_MAX;
		LIST_FOREACH(it, &(FreeMemBlocksList))
		{
			if(it->size==size)
			{
				LIST_REMOVE(&(FreeMemBlocksList),it);
				return it;
			}
			else if(it->size > size)
			{
				if(it->size < min_diff)
				{
					min_diff=it->size;
					temp = it;
				}
			}
		}
		if(min_diff==UINT_MAX)
		{
			return NULL;
		}
		else
		{
			struct MemBlock * newblock=AvailableMemBlocksList.lh_last;
			LIST_REMOVE(&(AvailableMemBlocksList),newblock);
			newblock->sva = temp->sva;
			newblock->size =size;
			temp->sva = temp->sva + size;
			temp->size=temp->size-size;
			return newblock;
		}
		return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
struct MemBlock *ptr=NULL;

struct MemBlock *alloc_block_NF(uint32 size)
{


		//TODO: [PROJECT MS1 - BONUS] [DYNAMIC ALLOCATOR] alloc_block_NF
		// Write your code here, remove the panic and write your code
		//panic("alloc_block_NF() is not implemented yet...!!");
	struct MemBlock * temp;
	if (ptr ==NULL)
	{
		//cprintf("1\n");
		LIST_FOREACH(temp, &(FreeMemBlocksList))
		{
			struct MemBlock foundblock ;
			if(temp->size==size)
			{
				ptr=temp->prev_next_info.le_next;
				LIST_REMOVE(&(FreeMemBlocksList),temp);
				return temp;
			}
			else if(temp->size > size)
			{
				struct MemBlock* newblock=AvailableMemBlocksList.lh_last;
				LIST_REMOVE(&(AvailableMemBlocksList),newblock);
				newblock->sva = temp->sva;
				newblock->size =size;
				temp->sva = temp->sva + size;
				temp->size=temp->size-size;
				ptr=temp;
				return newblock;
			}
		}
		return NULL;
	}
	else
	{

		//cprintf("%d \n",ptr->sva);
		LIST_FOREACH(temp, &(FreeMemBlocksList))
		{
			if (temp->sva >= ptr->sva)
			{
				//cprintf(">=\n");
				if(temp->size==size)
				{
					//cprintf("==\n");
					ptr=temp->prev_next_info.le_next;
					LIST_REMOVE(&(FreeMemBlocksList),temp);
					cprintf("%d \n",ptr->sva);
					return temp;
				}
				else if(temp->size > size)
				{
					//cprintf(">\n");
					//cprintf("%d \n",ptr->sva);
					struct MemBlock* newblock=AvailableMemBlocksList.lh_last;
					LIST_REMOVE(&AvailableMemBlocksList,newblock);
					//cprintf("%d \n",ptr->sva);
					newblock->sva = temp->sva;
					newblock->size =size;
					temp->sva = temp->sva + size;
					temp->size=temp->size-size;
					//cprintf("%d \n",newblock->size);
					ptr=temp;
					//cprintf("%d \n",newblock->sva);
					return newblock;
				}
				if(temp==LIST_LAST(&FreeMemBlocksList)){
					LIST_FOREACH(temp, &(FreeMemBlocksList))
						{
							struct MemBlock foundblock ;
							if(temp->size==size)
							{
								ptr=temp->prev_next_info.le_next;
								LIST_REMOVE(&(FreeMemBlocksList),temp);
								return temp;
							}
							else if(temp->size > size)
							{
								struct MemBlock* newblock=AvailableMemBlocksList.lh_last;
								LIST_REMOVE(&(AvailableMemBlocksList),newblock);
								newblock->sva = temp->sva;
								newblock->size =size;
								temp->sva = temp->sva + size;
								temp->size=temp->size-size;
								ptr=temp;
								return newblock;
							}
						}
						return NULL;
				}
			}
		}
		return NULL;
	}
	}
//===================================================
// [8] INSERT BLOCK (SORTED WITH MERGE) IN FREE LIST:
//===================================================

void insert_sorted_with_merge_freeList(struct MemBlock *blockToInsert)
{
	unsigned int size = LIST_SIZE(&FreeMemBlocksList);
	if (!size)
	{
		LIST_INSERT_HEAD(&FreeMemBlocksList, blockToInsert);
		return;
	}

	struct MemBlock *blockPrev = NULL;
	struct MemBlock *blockIterator;
	LIST_FOREACH(blockIterator, &FreeMemBlocksList)
	if (blockIterator->sva > blockToInsert->sva || blockIterator->sva == blockToInsert->sva)
		break;
	else
		blockPrev = blockIterator;

	if (blockPrev == NULL)
	{
		struct MemBlock *blockNew = LIST_FIRST(&FreeMemBlocksList);
		if ((blockToInsert->sva + blockToInsert->size) != blockNew->sva)
		{
			LIST_INSERT_HEAD(&FreeMemBlocksList, blockToInsert);
			return;
		}

		blockNew->sva = blockToInsert->sva;
		blockNew->size = blockToInsert->size + blockNew->size;
		blockToInsert->size = 0;
		blockToInsert->sva = 0;
		LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);

		return;
	}

	if (blockPrev->prev_next_info.le_next != NULL)
	{
		struct MemBlock *blockNext = blockPrev->prev_next_info.le_next;
		if (blockToInsert->sva == (blockPrev->sva + blockPrev->size) && (blockToInsert->sva + blockToInsert->size) == blockNext->sva)
		{
			blockPrev->size = blockPrev->size + blockToInsert->size + blockNext->size;
			LIST_REMOVE(&FreeMemBlocksList, blockNext);
			blockToInsert->sva = 0;
			blockToInsert->size = 0;
			LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
			blockNext->sva = 0;
			blockNext->size = 0;
			LIST_INSERT_HEAD(&AvailableMemBlocksList, blockNext);
			return;
		}

		if ((blockToInsert->sva + blockToInsert->size) == blockNext->sva)
		{
			blockNext->sva = blockToInsert->sva;
			blockNext->size += blockToInsert->size;
			blockToInsert->sva = 0;
			blockToInsert->size = 0;
			LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
			return;
		}

		if (blockToInsert->sva == (blockPrev->sva + blockPrev->size))
		{
			blockPrev->size += blockToInsert->size;
			blockToInsert->sva = 0;
			blockToInsert->size = 0;
			LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
			return;
		}

		blockNext->prev_next_info.le_prev = blockToInsert;
		blockToInsert->prev_next_info.le_next = blockNext;
		blockToInsert->prev_next_info.le_prev = blockPrev;
		blockPrev->prev_next_info.le_next = blockToInsert;
		return;
	}

	unsigned int endOfPrev = blockPrev->sva + blockPrev->size;
	if (blockToInsert->sva != (blockPrev->sva + blockPrev->size))
	{
		LIST_INSERT_TAIL(&FreeMemBlocksList, blockToInsert);
		return;
	}

	blockPrev->size += blockToInsert->size;
	blockToInsert->sva = 0;
	blockToInsert->size = 0;
	LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert);
}




