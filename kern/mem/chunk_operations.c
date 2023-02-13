/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include "kheap.h"
#include "memory_manager.h"


/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	uint32 src=ROUNDDOWN(source_va, PAGE_SIZE);
	uint32 dest =ROUNDDOWN(dest_va, PAGE_SIZE);
	uint32 * ptr_page_table=NULL;
	uint32 * ptr_page_table2=NULL;
	//uint32 * ptr_page_directory=NULL;
	for (int i = dest; i <dest+num_of_pages*PAGE_SIZE; i+= PAGE_SIZE)
	   {
			struct FrameInfo *ptr_frame_info=get_frame_info(page_directory,i,&ptr_page_table);
			//get_page_table(page_directory,i,&ptr_page_table);
			if(ptr_frame_info!=NULL)
			{
			//	if(ptr_page_table[PTX(i)] != 0)
					return -1;
			}
	   }
	   for (int i = dest; i <dest+num_of_pages*PAGE_SIZE ; i+= PAGE_SIZE)
	   {
	    //cprintf("loop \n");

		 get_page_table(page_directory,i,&ptr_page_table);
		 if(ptr_page_table==NULL)
		 {
			 create_page_table(page_directory,i);
			// cprintf("1 \n");
			 get_page_table(page_directory,i,&ptr_page_table);
		  }
		 get_page_table(page_directory,src,&ptr_page_table2);

		 struct FrameInfo* frame = get_frame_info(page_directory,src,&ptr_page_table2);

		 map_frame(page_directory,frame,i,pt_get_page_permissions(page_directory,src));
		 ptr_page_table[PTX(i)]=ptr_page_table2[PTX(src)];

		 unmap_frame(page_directory, src);

		 src+=PAGE_SIZE;

	    }
	return 0;
	}



//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the panic and write your code
	//panic("copy_paste_chunk() is not implemented yet...!!");
	uint32 x =source_va;
uint32 y =dest_va;
uint32 a=dest_va+size;
uint32 b=source_va+size;
uint32 * ptr_page_table=NULL;
uint8* src=(uint8*)source_va;
uint8* dest=(uint8*)dest_va;


 for(int i=y;i<a;i++){
	 struct FrameInfo *ptr_frame_info=get_frame_info(page_directory,i,&ptr_page_table);
	 int perms=pt_get_page_permissions(page_directory,i);
if(ptr_frame_info!=NULL)
{
	if((perms&PERM_WRITEABLE)!=PERM_WRITEABLE)
		return -1;
}
}
		for (int i = y; i <a ; i++)
		{ int perms=pt_get_page_permissions(page_directory,x);
		 //cprintf("loop \n");

		 get_page_table(page_directory,i,&ptr_page_table);
		 if(ptr_page_table==NULL)
		 {
			 create_page_table(page_directory,i);
			 //cprintf("1 \n");
			 //get_page_table(page_directory,i,&ptr_page_table);
		 }
		 struct FrameInfo *ptr_frame_info=get_frame_info(page_directory,i,&ptr_page_table);
		 if(ptr_frame_info==NULL){
		 int ret= allocate_frame(&ptr_frame_info);
			// cprintf("2 \n");

		 map_frame(page_directory,ptr_frame_info,i,PERM_WRITEABLE);
		 //cprintf("3\n");
		 }

		 if((perms&PERM_USER)==PERM_USER)
		 pt_set_page_permissions(page_directory,i,PERM_USER,0);
		 x++;
		 }

	   for(int i=y;i<a;i++)
	   { //cprintf("4\n");
		   *dest=*src;
			src++;
			dest++;
	   }

	   return 0;
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from dest_va with the source_va
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the panic and write your code
	//panic("share_chunk() is not implemented yet...!!");
	uint32 x =ROUNDDOWN(source_va, PAGE_SIZE);
	   uint32 y =ROUNDDOWN(dest_va, PAGE_SIZE);
	   uint32 a=ROUNDUP(dest_va+size,PAGE_SIZE);
	   uint32 b=ROUNDUP(source_va+size,PAGE_SIZE);
	   uint32 * ptr_page_table=NULL;
	   uint32 * ptr_page_table2=NULL;
		   for(int i = y; i < a; i+= PAGE_SIZE){

			   			get_page_table(page_directory,i,&ptr_page_table);
			   			if(ptr_page_table!=NULL)
			   			{
			   				if(ptr_page_table[PTX(i)] != 0)
			   					return -1;
			   			}
			   	   }
			   	   for (int i = y; i <a ; i+= PAGE_SIZE)
			   	   {
			   	    //cprintf("loop \n");

			   		 get_page_table(page_directory,i,&ptr_page_table);
			   		 if(ptr_page_table==NULL)
			   		 {
			   			 create_page_table(page_directory,i);
			   			// cprintf("1 \n");
			   			 get_page_table(page_directory,i,&ptr_page_table);
			   		  }

	    get_page_table(page_directory,x,&ptr_page_table2);
	  //  ptr_page_table[PTX(i)]= ptr_page_table2[PTX(x)];
	    struct FrameInfo* frame = get_frame_info(page_directory,x,&ptr_page_table2);
				//  pt_set_page_permissions(page_directory,i,perms,0);
				  map_frame(page_directory,frame,i,perms);

			      x+=PAGE_SIZE;

			   	    }


		   	return 0;


}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate in RAM the given range [va, va+size)
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] allocate_chunk
	// Write your code here, remove the panic and write your code
	//panic("allocate_chunk() is not implemented yet...!!");
		uint32 x =ROUNDDOWN(va, PAGE_SIZE);
		uint32 y =ROUNDUP(va+size, PAGE_SIZE);
		uint32 * ptr_pg_table=NULL;
		for(int i = x; i < y; i+= PAGE_SIZE)
		{
			//cprintf("5");
			struct FrameInfo *ptr_f_info=get_frame_info(page_directory,i,&ptr_pg_table);
			//cprintf("eh");
			if(ptr_f_info!=NULL)
			{

				//cprintf("5 \n");
				//uint32 pa = to_physical_address(ptr_f_info);
				//cprintf("%x" , ptr_f_info);
				//struct FrameInfo *ptr_f_pa_info=to_frame_info(pa);
				//cprintf("num of refs at pa %x = %d\n", pa, ptr_f_pa_info->references);
				return -1;
			}

			int ret= allocate_frame(&ptr_f_info);
			if(ret!=E_NO_MEM)
			{
				map_frame(page_directory,ptr_f_info,i,perms);
			}
		/*	if (ret == E_NO_MEM)
			{
				cprintf("No enough memory for page table!\n");
				//free the allocated frame
				free_frame(ptr_f_info) ;
			}*/
			ptr_f_info->va=i;

		}
		for (int i = x; i < y; i+= PAGE_SIZE)
		{
			//cprintf("5");
			get_page_table(page_directory,i,&ptr_pg_table);
			if(ptr_pg_table==NULL)
			//	cprintf("30");
				create_page_table(page_directory,va);
		}
		return 0;
}

/*BONUS*/
//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	//panic("calculate_allocated_space() is not implemented yet...!!");
	uint32 *ptr_pg_tbl=NULL;
	uint32 a = ROUNDDOWN(sva  ,PAGE_SIZE);
	uint32 b = ROUNDUP(eva,PAGE_SIZE);
	uint32  c =0;
	uint32 * x=NULL;


	for(int i=a ; i< b; i+=PAGE_SIZE)
	{
		get_page_table(page_directory,i,&ptr_pg_tbl);
			if(ptr_pg_tbl!=NULL)
				if(x!=ptr_pg_tbl)
					c++;
		x=ptr_pg_tbl;
	}
	//cprintf("c =  %d \n",c);
	*num_tables=c;

	uint32  y =0;

	struct FrameInfo *perv=NULL;
	for(int i=a ; i< b ; i+=PAGE_SIZE)
	{

		//get_page_table(page_directory,i,&ptr_pg_tbl);
		struct FrameInfo * ptr_info=get_frame_info(page_directory,i,&ptr_pg_tbl);
		if(ptr_info !=NULL)
			if(ptr_info!=perv)
			y++;
		perv=ptr_info;
	}
	//cprintf("y =  %d \n",y);
	*num_pages=y;

		//uint32  x =0;
		//int ret=0;
	//cprintf("pages =  %d \n",*num_pages);
	//cprintf("tables =  %d \n",*num_tables);


}

/*BONUS*/
//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
// calculate_required_frames:
// calculates the new allocation size required for given address+size,
// we are not interested in knowing if pages or tables actually exist in memory or the page file,
// we are interested in knowing whether they are allocated or not.
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	// Write your code here, remove the panic and write your code
	panic("allocate_user_mem() is not implemented yet...!!");
}


//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3] [USER HEAP - KERNEL SIDE] free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");
	//TODO: [PROJECT MS3] [USER HEAP - KERNEL SIDE] free_user_mem
		// Write your code here, remove the panic and write your code
		//panic("free_user_mem() is not implemented yet...!!");
		//This function should:
		//1. Free ALL pages of the given range from the Page File

		uint32 va=ROUNDDOWN(virtual_address,PAGE_SIZE);
		size = ROUNDUP(size,PAGE_SIZE);
		uint32 max= virtual_address + size ;
		for(uint32 i=va; i<max ;i+=PAGE_SIZE)
		{
			pf_remove_env_page(e,i);
			env_page_ws_invalidate(e,i);
			unmap_frame(e->env_page_directory,i);

			uint32 * pt=NULL;

			   bool flag = 1;
			   get_page_table(e->env_page_directory,i,&pt);
			   if(pt != NULL)
			  {
				for(int j =0 ; j<1024 ;j++)
				{
					if(pt[j]!=0)
						flag=0;
				}
				if(flag==1)
				{
					e->env_page_directory[PDX(i)] = 0 ;
						tlbflush();
					kfree((void*)pt);
				}
			}
		}
}

//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Free any BUFFERED pages in the given range
	//4. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3 - BONUS] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");

	// This function should move all pages from "src_virtual_address" to "dst_virtual_address"
	// with the given size
	// After finished, the src_virtual_address must no longer be accessed/exist in either page file
	// or main memory

	/**/
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

