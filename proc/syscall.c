/*
 * System calls.
 *
 * Copyright (C) 2003 Juha Aatrokoski, Timo Lilja,
 *   Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: syscall.c,v 1.3 2004/01/13 11:10:05 ttakanen Exp $
 *
 */
#include "kernel/cswitch.h"
#include "proc/syscall.h"
#include "kernel/halt.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "kernel/assert.h"
#include "drivers/device.h"
#include "drivers/yams.h"
#include "drivers/gcd.h"
#include "lib/debug.h"
#include "proc/process.h"
#include "kernel/thread.h"
#include "vm/vm.h"
#include "vm/pagepool.h"
#include "kernel/interrupt.h"

int syscall_write(int fhandle, const void *buffer, int length){
  device_t *dev;
  gcd_t *gcd;

  dev = device_get(YAMS_TYPECODE_TTY, 0);
  if(dev == NULL)
    return -1;

  gcd = (gcd_t *)dev->generic_device;
  if(gcd == NULL)
    return -1;

  if (fhandle != FILEHANDLE_STDOUT) {
    KERNEL_PANIC("Can only write to standard output!");
  }

  return gcd->write(gcd, buffer, length);
}

int syscall_read(int fhandle, void *buffer, int length){
  device_t *dev;
  gcd_t *gcd;
  
  dev = device_get(YAMS_TYPECODE_TTY, fhandle);
  if(dev == NULL)
    return -1;

  gcd = (gcd_t *)dev->generic_device;
  if(gcd == NULL)
    return -1;

  if (fhandle != FILEHANDLE_STDIN) {
    KERNEL_PANIC("Can only read from standard input!");
  }

  return gcd->read(gcd, buffer, length);
}

int syscall_exec(const char *filename)
{

  return process_spawn(filename);
}

void syscall_exit(int retval)

{
  process_finish(retval);
}

int syscall_join(int pid)
{
  return process_join(pid);
}

void *syscall_memlimit (void* new_heap_end)
{
  int i;
  uint32_t heap_end = process_get_current_process_entry()->heap_end;
  int pages;
  interrupt_status_t intr_status;
  pagetable_t *pagetable;
  uint32_t phys_addr;

  /* If argument is NULL, return the current heap end address. */
  if(new_heap_end == NULL){
    DEBUG("debug_G4", "Syscall_memlimit: new_heap_end == NULL, current heap_end is %d\n",heap_end);
    return (void*)process_get_current_process_entry()->heap_end;
  }

  DEBUG("debug_G4", "Syscall_memlimit: heap_end is %d and arg new_heap_end is %d\n",heap_end,(uint32_t)new_heap_end);
  
  /* As unmap is not implemented, it is not allowed to decrease the heap. */
  KERNEL_ASSERT(heap_end < (uint32_t)new_heap_end);
  
  /* Calculate the needed number of pages to fullfill the request.*/
  pages = (uint32_t)new_heap_end/PAGE_SIZE - heap_end/PAGE_SIZE;
  KERNEL_ASSERT(pages > -1);
  DEBUG("debug_G4","Syscall_memlimit: Number of needed pages: %d\n", pages);

  /* Disable interrupts to make sure no one steals any pages we might need. */
  intr_status = _interrupt_disable();

  /* If the amount of free pages can fullfill our request,
     we want to assume that we can take those without having to
     worry. Hence the disabling of interrupts. */
  if (pagepool_get_num_free_pages() < pages) {
    DEBUG("debug_G4","Syscall_memlimit: Not enough free pages are available\n");
    _interrupt_set_state(intr_status);
    return NULL;
  }

  pagetable = thread_get_current_thread_entry()->pagetable;

  KERNEL_ASSERT(pagetable != NULL);

  /* Make sure that the pagetable has room for the needed
     number of pages. */
  if ((pagetable->valid_count+pages) > PAGETABLE_ENTRIES) {
    DEBUG("debug_G4","Syscall_memlimit: Request is too big. Cannot add pages %d to pagetable of thread ASID %d\n",pages,pagetable->ASID);
    _interrupt_set_state(intr_status);
    return NULL;
  }

  /* Make the tmp heap_end value point to the next page address,
     to avoid mapping with the same virtual address, if the heap was
     at a page-boundary. And to keep the virtual addresses page-aligned.
     This has only any effect if we need to allocate atleast one page. */
  if ((heap_end % PAGE_SIZE) == 0) {
    heap_end = heap_end + PAGE_SIZE;
  } else {
    heap_end = heap_end + (heap_end % PAGE_SIZE);
  }

  /* Get the number of needed pages and map them into the
     process page table. */
  for(i = 0; i < pages; i++){
    phys_addr = pagepool_get_phys_page();
    if (!phys_addr)
      KERNEL_PANIC("Syscall_memlimit: No physical page left, should not be able to happen.");
    vm_map(pagetable, phys_addr, heap_end + i * PAGE_SIZE, 1);
  }
  _interrupt_set_state(intr_status);

  /* Update the process heap pointer. */
  process_get_current_process_entry()->heap_end = (uint32_t)new_heap_end;
  
  /* Return the updated heap pointer. */
  return (void*)process_get_current_process_entry()->heap_end;
}

/**
 * Handle system calls. Interrupts are enabled when this function is
 * called.
 *
 * @param user_context The userland context (CPU registers as they
 * where when system call instruction was called in userland)
 */
void syscall_handle(context_t *user_context)
{
    /* When a syscall is executed in userland, register a0 contains
     * the number of the syscall. Registers a1, a2 and a3 contain the
     * arguments of the syscall. The userland code expects that after
     * returning from the syscall instruction the return value of the
     * syscall is found in register v0. Before entering this function
     * the userland context has been saved to user_context and after
     * returning from this function the userland context will be
     * restored from user_context.
     */
    switch(user_context->cpu_regs[MIPS_REGISTER_A0]) {
    case SYSCALL_HALT:
      halt_kernel();
      break;
    case SYSCALL_WRITE:
      user_context->cpu_regs[MIPS_REGISTER_V0] = syscall_write((int)user_context->cpu_regs[MIPS_REGISTER_A1], 
		    (void*)user_context->cpu_regs[MIPS_REGISTER_A2],
		    (int)user_context->cpu_regs[MIPS_REGISTER_A3]);
      break;
    case SYSCALL_READ:
      user_context->cpu_regs[MIPS_REGISTER_V0] = syscall_read((int)user_context->cpu_regs[MIPS_REGISTER_A1], 
		   (void*)user_context->cpu_regs[MIPS_REGISTER_A2],
		   (int)user_context->cpu_regs[MIPS_REGISTER_A3]);
      break;
    case SYSCALL_EXEC:
      user_context->cpu_regs[MIPS_REGISTER_V0] = syscall_exec((char*)user_context->cpu_regs[MIPS_REGISTER_A1]);
      break;
    case SYSCALL_EXIT:
      syscall_exit((int)user_context->cpu_regs[MIPS_REGISTER_A1]);
      break;
    case SYSCALL_JOIN:
      user_context->cpu_regs[MIPS_REGISTER_V0] = syscall_join((int)user_context->cpu_regs[MIPS_REGISTER_A1]);
      break;
    case SYSCALL_MEMLIMIT:
      user_context->cpu_regs[MIPS_REGISTER_V0] = (uint32_t)syscall_memlimit((void*)user_context->cpu_regs[MIPS_REGISTER_A1]);
      break;
    default: 
      KERNEL_PANIC("Unhandled system call\n");
    }

    /* Move to next instruction after system call */
    user_context->pc += 4;
}
