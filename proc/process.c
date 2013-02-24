/*
 * Process startup.
 *
 * Copyright (C) 2003-2005 Juha Aatrokoski, Timo Lilja,
 *       Leena Salmela, Teemu Takanen, Aleksi Virtanen.
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
 * $Id: process.c,v 1.11 2007/03/07 18:12:00 ttakanen Exp $
 *
 */

#include "proc/process.h"
#include "proc/elf.h"
#include "kernel/thread.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "fs/vfs.h"
#include "drivers/yams.h"
#include "vm/vm.h"
#include "vm/pagepool.h"
#include "kernel/spinlock.h"

/** @name Process startup
 *
 * This module contains facilities for managing userland process.
 */

process_control_block_t process_table[PROCESS_MAX_PROCESSES];

spinlock_t process_table_slock;


/**
 * Starts one userland process. The thread calling this function will
 * be used to run the process and will therefore never return from
 * this function. This function asserts that no errors occur in
 * process startup (the executable file exists and is a valid ecoff
 * file, enough memory is available, file operations succeed...).
 * Therefore this function is not suitable to allow startup of
 * arbitrary processes.
 *
 * @executable The name of the executable to be run in the userland
 * process
 */
/*void process_start(const char *executable)*/
void process_start(process_id_t pid)
{
    thread_table_t *my_entry;
    pagetable_t *pagetable;
    uint32_t phys_page;
    context_t user_context;
    uint32_t stack_bottom;
    elf_info_t elf;
    openfile_t file;

    int i;

    interrupt_status_t intr_status;
    const char *executable;
    
    KERNEL_ASSERT(pid >= 0);
    KERNEL_ASSERT(pid < PROCESS_MAX_PROCESSES);

    intr_status = _interrupt_disable();
    spinlock_acquire(&process_table_slock);

    executable = process_get_process_entry(pid)->executable;

    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    kprintf("Process_start found this executable: %s\n", executable);
    /* Is used by process_spawn.
     * This must take a pid instead of string, it can the look
     * up in the process_table to get the executable. */

    my_entry = thread_get_current_thread_entry();

    /* If the pagetable of this thread is not NULL, we are trying to
       run a userland process for a second time in the same thread.
       This is not possible. */
    KERNEL_ASSERT(my_entry->pagetable == NULL);

    pagetable = vm_create_pagetable(thread_get_current_thread());
    KERNEL_ASSERT(pagetable != NULL);

    intr_status = _interrupt_disable();
    my_entry->pagetable = pagetable;
    _interrupt_set_state(intr_status);

    file = vfs_open((char *)executable);
    /* Make sure the file existed and was a valid ELF file */
    KERNEL_ASSERT(file >= 0);
    KERNEL_ASSERT(elf_parse_header(&elf, file));

    /* Trivial and naive sanity check for entry point: */
    KERNEL_ASSERT(elf.entry_point >= PAGE_SIZE);

    /* Calculate the number of pages needed by the whole process
       (including userland stack). Since we don't have proper tlb
       handling code, all these pages must fit into TLB. */
    KERNEL_ASSERT(elf.ro_pages + elf.rw_pages + CONFIG_USERLAND_STACK_SIZE
		  <= _tlb_get_maxindex() + 1);

    /* Allocate and map stack */
    for(i = 0; i < CONFIG_USERLAND_STACK_SIZE; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - i*PAGE_SIZE, 1);
    }

    /* Allocate and map pages for the segments. We assume that
       segments begin at page boundary. (The linker script in tests
       directory creates this kind of segments) */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.ro_vaddr + i*PAGE_SIZE, 1);
    }

    for(i = 0; i < (int)elf.rw_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.rw_vaddr + i*PAGE_SIZE, 1);
    }

    /* Put the mapped pages into TLB. Here we again assume that the
       pages fit into the TLB. After writing proper TLB exception
       handling this call should be skipped. */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);
    
    /* Now we may use the virtual addresses of the segments. */

    /* Zero the pages. */
    memoryset((void *)elf.ro_vaddr, 0, elf.ro_pages*PAGE_SIZE);
    memoryset((void *)elf.rw_vaddr, 0, elf.rw_pages*PAGE_SIZE);

    stack_bottom = (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - 
        (CONFIG_USERLAND_STACK_SIZE-1)*PAGE_SIZE;
    memoryset((void *)stack_bottom, 0, CONFIG_USERLAND_STACK_SIZE*PAGE_SIZE);

    /* Copy segments */

    if (elf.ro_size > 0) {
	/* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.ro_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.ro_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.ro_vaddr, elf.ro_size)
		      == (int)elf.ro_size);
    }

    if (elf.rw_size > 0) {
	/* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.rw_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.rw_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.rw_vaddr, elf.rw_size)
		      == (int)elf.rw_size);
    }


    /* Set the dirty bit to zero (read-only) on read-only pages. */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        vm_set_dirty(my_entry->pagetable, elf.ro_vaddr + i*PAGE_SIZE, 0);
    }

    /* Insert page mappings again to TLB to take read-only bits into use */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);

    /* Initialize the user context. (Status register is handled by
       thread_goto_userland) */
    memoryset(&user_context, 0, sizeof(user_context));
    user_context.cpu_regs[MIPS_REGISTER_SP] = USERLAND_STACK_TOP;
    user_context.pc = elf.entry_point;

    thread_goto_userland(&user_context);

    KERNEL_PANIC("thread_goto_userland failed.");
}

void process_init() {
  /* Initialize the process table, fill it with empty entries. */
  int i;
  spinlock_reset(&process_table_slock);

  for (i=0; i < PROCESS_MAX_PROCESSES; i++)
  {
    /* process_table[i].executable = "";  Nedded ?*/
    process_table[i].process_state = FREE;
    process_table[i].process_id = -1;
    process_table[i].sleeps_on = 0;
  }
}

void process_help_spawn(uint32_t arg)
{
  process_start((process_id_t)arg);
}

process_id_t process_spawn(const char *executable) {
  /* Check if there is room for one more process
   * in the process table, if there is room:
   * take that spot, init process, mark as child of this
   * return its newly created pid.
   * Must call process_start.*/

  /* The pid is the process's index in the process_table. */
  
  int i;
  int k;
  interrupt_status_t intr_status;
  TID_t tid;
  kprintf("Trying to spawn process with executable: %s\n", (char*)executable);
  /* We must first disable interrups and acquire the process_table lock. */
  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  /* Find empty spot in process_table. */
  for (i=0; i < PROCESS_MAX_PROCESSES; i++)
  {
    if (process_table[i].process_id == -1) break;
  }
  if (i == PROCESS_MAX_PROCESSES) /* The process_table is filled. */
  {
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    return -1;
  }

  /* i contains the new process's process_id and the index for process_table. */
  /* process_table[i].executable = executable;*/
  for (k=0; k < PROCESS_MAX_PROCESSES; k++)
  {
    process_table[i].executable[k] = *executable;
    if (*executable == '\0') break;
    executable++;
  }
  /* executable = executable;*/
  process_table[i].process_state = READY;
  process_table[i].process_id = i;
  process_table[i].sleeps_on = 0;

  /* Enable interrups again, and release the process_table lock. */
  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  /* This is still parent process, so we must create new thread and let 
   * the spawned process run in that, and let that call process_start. */

  /* Create new thread, give it the new process, run process_start. */
  kprintf("made it here1113333331111122\n");
  if (i == 0) /* If i is 0, then I assume it is the initproc.
		 And therefore should not run in a new thread. */
  {
    process_start(i);
  }
  kprintf("made it here1111111122\n");
  tid = thread_create(process_help_spawn, (uint32_t)i);
  /* set process variable in the new process? */

  KERNEL_ASSERT(tid >= 0); /* If no thread could be created-> full thread_table->
			   * tid is negative, prob'ly should use something more
			   * relaxed than KERNEL_ASSERT. */

  /*spinlock_acquire(&thread_table_slock);*/
  thread_get_thread_entry(tid)->process_id = i;
  /* spinlock_release(&thread_table_slock);*/

  thread_run(tid); /* Rigtig måde at gøre det på? */
  /* thread_switch();*/ /* Rigtig måde at gøre det på? Tror ikke man skal switche. */

  return i;
}

/* Stop the process and the thread it runs in. Sets the return value as well */
void process_finish(int retval) {
  retval=retval;
  /* Stop the process and the thread it runs in.
   * Set the return value, retval.
   * Wake up any joined threads.
   * Set state to zombie. */

  interrupt_status_t intr_status;
  process_id_t pid;

  intr_status = _interrupt_disable();

  spinlock_acquire(&process_table_slock);
  /*spinlock_acquire(&thread_table_slock);*/

  pid = thread_get_current_thread_entry()->process_id;
  /* process_table[pid].executable = ""; */
  process_table[pid].process_state = ZOMBIE;
  process_table[pid].process_id = -1;
  process_table[pid].sleeps_on = 0;

  spinlock_release(&process_table_slock);

  /* Before calling thread finish, process_finish must do:
   * vm_destroy_pagetable(thr->pagetable);
   * thr->pagetable = NULL;
   * Where thr is the kernel thread this process is executing on.
   * The two lines cleans on virtuel memory used. */

  vm_destroy_pagetable(thread_get_current_thread_entry()->pagetable);
  thread_get_current_thread_entry()->pagetable = NULL;

  /* spinlock_release(&thread_table_slock);*/

  _interrupt_set_state(intr_status);
  thread_finish();
}

int process_join(process_id_t pid) {
  pid=pid;

  /* Look pid up in table, if pid exists,
   * make sure it is a child of this process aswell,
   * wait/spin/sleep (use buenos sleep queues) until it dies,
   * and remove it from the table, when it returns.
   * Set state to running (?) */
  KERNEL_PANIC("Not implemented.");
  return 0; /* Dummy */
}


process_id_t process_get_current_process(void)
{
    return thread_get_current_thread_entry()->process_id;
}

process_control_block_t *process_get_current_process_entry(void)
{
    return &process_table[process_get_current_process()];
}

process_control_block_t *process_get_process_entry(process_id_t pid) {
    return &process_table[pid];
}


/** @} */
