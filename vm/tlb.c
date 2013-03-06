/*
 * TLB handling
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
 * $Id: tlb.c,v 1.6 2004/04/16 10:54:29 ttakanen Exp $
 *
 */

#include "kernel/panic.h"
#include "kernel/assert.h"
#include "vm/tlb.h"
#include "vm/pagetable.h"
#include "kernel/thread.h"
#include "lib/debug.h"
#include "proc/process.h"

void tlb_modified_exception(void)
{
  tlb_exception_state_t tlb_exc_state;
  _tlb_get_exception_state(&tlb_exc_state);

  thread_table_t *current_thread = thread_get_current_thread_entry();

  if (current_thread->pagetable == NULL) {
    KERNEL_PANIC("Unhandled TLB modified exception");
  }

  /* kill user process */
  process_finish(-6);
}

void tlb_load_exception(void)
{
  DEBUG("debug_G4", "############00\n");
  tlb_exception_state_t tlb_exc_state;
  _tlb_get_exception_state(&tlb_exc_state);
  DEBUG("debug_G4", "############1\n");

  thread_table_t *current_thread = thread_get_current_thread_entry();
  DEBUG("debug_G4", "############2\n");

  // DEBUG("debug_G4","thread_get_c.. = %d\ntlb_ex.. = %d\n", thread_get_current_thread(), tlb_exc_state.asid);
  // DEBUG("debug_G4","thread_get_c.. = %d\ntlb_ex.. = %d\n", (unsigned int)thread_get_current_thread(), (unsigned int)tlb_exc_state.asid);

 // KERNEL_ASSERT((unsigned int)*current_thread == (unsigned int)tlb_exc_state.asid);
  DEBUG("debug_G4", "############3\n");

  if (current_thread->pagetable == NULL) {
    KERNEL_PANIC("In kernel mode, TLB load exception");
  }
  DEBUG("debug_G4", "############4\n");

  /*tlb_entry_t tlb_entry = current_thread->pagetable->entries[tlb_exc_state->badvpn2];
   */
  int i;
  tlb_entry_t tlb_entry;
  tlb_entry_t tmp;
  
  for (i=0; i < PAGETABLE_ENTRIES; i++) {
    tmp = current_thread->pagetable->entries[i];
    if ((unsigned int)tmp.VPN2 == (unsigned int)tlb_exc_state.badvpn2) {
      DEBUG("debug_G4", "Found match in tlb_load_exception\n");
      tlb_entry = tmp;
      break;
    }
  }
  
  if (i == PAGETABLE_ENTRIES) {
    KERNEL_PANIC("tlb_load_exception, page does not exist");
  }

  _tlb_write_random(&tlb_entry);


  //  KERNEL_PANIC("Unhandled TLB load exception");
}

void tlb_store_exception(void)
{
  tlb_load_exception();
  //    KERNEL_PANIC("Unhandled TLB store exception");
}

/**
 * Fill TLB with given pagetable. This function is used to set memory
 * mappings in CP0's TLB before we have a proper TLB handling system.
 * This approach limits the maximum mapping size to 128kB.
 *
 * @param pagetable Mappings to write to TLB.
 *
 */
/*
void tlb_fill(pagetable_t *pagetable)
{
    if(pagetable == NULL)
	return;

    / Check that the pagetable can fit into TLB. This is needed until
     we have proper VM system, because the whole pagetable must fit
     into TLB. /
    KERNEL_ASSERT(pagetable->valid_count <= (_tlb_get_maxindex()+1));

    _tlb_write(pagetable->entries, 0, pagetable->valid_count);

    / Set ASID field in Co-Processor 0 to match thread ID so that
       only entries with the ASID of the current thread will match in
       the TLB hardware. /
    _tlb_set_asid(pagetable->ASID);
}
*/
