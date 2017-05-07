//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 pagestart;
static int nfreepages;
static int freemapmax;

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
    int i ;
    //int startaddr;
    int startaddr = (lastosaddress >> MEM_L2FIELD_FIRST_BITNUM);
    //startaddr = MemoryGetSize();
    freemapmax = 512;
    for(i = 0; i < freemapmax; i+=32){
        if(i > startaddr) freemap[i/32] = 0xFFFFFFFF;
        else if(startaddr - i < 32){
            freemap[i/32] = 0xFFFFFFFF - (1 << (startaddr - i + 1)) + 1;
        }
        else freemap[i/32] = 0;
    }
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
    uint32 pageid;
    uint32 pte;
    uint32 valid;
    uint32 phyaddr;
    uint32 masked;
    uint32* l2addr;
    int ppagenum;
//   printf("%d addr: %x %x %x %x %x\n", findpid(pcb), addr, l2addr, pte, pageid, pcb);

    masked = (1 << MEM_L2FIELD_FIRST_BITNUM) - 1;

    pageid = addr >> MEM_L1FIELD_FIRST_BITNUM;
    l2addr = pcb->pagetable[pageid & 0x3];
//    printf("data: %x, masked: %x\n", l2addr[(addr >> MEM_L2FIELD_FIRST_BITNUM) & (0x3F)], masked );
    pte = l2addr[((addr >> MEM_L2FIELD_FIRST_BITNUM) & (0x03F))];
//    printf("addr: %x %x %x %x %x\n", addr, l2addr, pte, pageid, (addr >> MEM_L2FIELD_FIRST_BITNUM) & (0x3F));
    valid = pte & MEM_PTE_VALID;
    if(valid != 0){
        phyaddr = (pte & (0xFFFFFFFF - masked)) | (addr & masked);
    }
    else{
        MemoryPageFaultHandler(pcb);
        //ppagenum = MemoryAllocPage(); 
        //l2addr[(addr >> MEM_L2FIELD_FIRST_BITNUM) & (0x3F)] = MemorySetupPte(ppagenum);
        //pte = l2addr[(addr >> MEM_L2FIELD_FIRST_BITNUM) & (0x3F)];
        //pte =  pcb->pagetable[pageid];
        phyaddr = (pte & (0xFFFFFFFF - masked)) | (addr & masked);

    }
//    printf("Phyaddr: %x\n", phyaddr); 
//    printf("%d End addr: %x %x %x %x %x\n", findpid(pcb), addr, l2addr, pte, pageid, (addr >> MEM_L2FIELD_FIRST_BITNUM) & (0x3F));
    return phyaddr;


}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);
    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  //printf("Return\n");
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
  int ppagenum;
  uint32* l2addr; 
//printf("MemoryPageFault\n");
  /* uint32 addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT]; */
  if(pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]/MEM_PAGESIZE >  pcb->currentSavedFrame[PROCESS_STACK_FAULT]/MEM_PAGESIZE){
    printf("FATAL ERROR (%d): segmentation fault at page address %x\n", findpid(pcb), pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]); 
    ProcessKill(); 
    return MEM_FAIL;
  }
  /* // segfault if the faulting address is not part of the stack */
  /* if (vpagenum < stackpagenum) { */
  /*   dbprintf('m', "addr = %x\nsp = %x\n", addr, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]); */
  /*   printf("FATAL ERROR (%d): segmentation fault at page address %x\n", findpid(pcb), addr); */
  /*   ProcessKill(); */
  /*   return MEM_FAIL; */
  /* } */
//  if(pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER] <  pcb->currentSavedFrame[PROCESS_STACK_FAULT]){
//    return MEM_FAIL;
//  }
  ppagenum = MemoryAllocPage(); 
  l2addr = pcb->pagetable[(int)pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM];
  l2addr[((int)pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L2FIELD_FIRST_BITNUM) & (0x3f)] = MemorySetupPte(ppagenum); 
//printf("CurrentSavedFrame: %x\n", (int)pcb->currentSavedFrame[PROCESS_STACK_FAULT] );
//printf("New L1 page id: %x\n", (int)pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM);
//printf("New L2 page id: %x\n", ((int)pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L2FIELD_FIRST_BITNUM) & (0x3f));
  /* dbprintf('m', "Returning from page fault handler\n"); */
  return MEM_SUCCESS; 
 // return MEM_FAIL;
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) {

  int free32 = 0;
  uint32 i;
  uint32 temp = 1;
  int freepage = 0;
 // printf("Allocate Page\n");
  for(free32 = 0; free32 < freemapmax/32; free32++){
    //printf("free32: %d, freemap: %x\n", free32, freemap[free32]);
    if(freemap[free32] != 0) break;
  }
  //printf("free32, %d\n", free32);
  for(i = 0; i < 32; i++){
    if((freemap[free32] & (temp << i)) != 0) {
      if(i == 31) freemap[free32] = 0;
      else freemap[free32] &= (~(temp << i));
      break;
    }
  }

  //printf("i, %d\n", i);
  freepage = free32 << 5 | i;


  return freepage;
}


uint32 MemorySetupPte (uint32 page) {
  uint32 pte = 0;
  pte = page << MEM_L2FIELD_FIRST_BITNUM;
  pte |= MEM_PTE_VALID;
  return pte;
}


void MemoryFreePage(uint32 page) {
  int free;
  int free32;
  free = page >> MEM_L2FIELD_FIRST_BITNUM;
  free32 = free >> 5;
  free = free % 32;
  freemap[free32] |= (1 << free);
}

