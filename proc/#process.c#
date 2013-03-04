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
#include "kernel/sleepq.h"
#include "lib/debug.h"
#include "lib/libc.h"

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

    /* Make sure the given pid is correct.
       A wrong pid is not allowed in here. */
    KERNEL_ASSERT(pid >= 0);
    KERNEL_ASSERT(pid < PROCESS_MAX_PROCESSES);

    /* Lock the process_table lock and disable interrupts. */
    intr_status = _interrupt_disable();
    spinlock_acquire(&process_table_slock);

    /* Fetch the executable string from the process_table. */
    executable = process_get_process_entry(pid)->executable;

    /* Release the lock and re-establish the interrupt status. */
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);

    DEBUG("process_Debug","Debug: process_start found this executable: %s\n", executable);

    /* Is used by process_spawn.
     * This must take a pid instead of string, it can the look
     * up in the process_table to get the executable. */

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

/* Marks a given entry in the process_table to be empty. */
void process_mark_process_table_entry_empty(process_id_t i)
{
  /* What only really matters is that process_id is set to -1.
     We just want to try and maintain a 'clean' table, and remove any trash. */
  stringcopy(process_table[i].executable, "", MAX_FILE_NAME);
  process_table[i].process_state = WAITING;
  process_table[i].process_id = -1;
  process_table[i].retval = 0;
  process_table[i].parent_pid = -1;
}

/* Initialize the process_table spinlock and the process_table.
   Must be called during kernel initialization. */
void process_init() {
  int i;

  /* Reset/initialize the process_table spinlock. */
  spinlock_reset(&process_table_slock);

  /* Initialize the process table by filling it with 'empty' process_control_blocks.
     An empty entry is denoted by that process_id is -1. */
  for (i=0; i < PROCESS_MAX_PROCESSES; i++)
  {
    process_mark_process_table_entry_empty(i);
  }
}

/* A helper function, to avoid type warnings/errors by the compiler. */
void process_help_spawn(uint32_t arg)
{
  process_start((process_id_t)arg);
}

/* Create a new process in a new thread.
   param executable, a string containing the file name.
   return i, the process id of the created process.
   return negative if any errors occoured.
          -1 if the process_table is already full.
	  -2 if no thread could be created. */
process_id_t process_spawn(const char *executable) {
  int i;
  interrupt_status_t intr_status;
  TID_t tid;

  DEBUG("process_Debug","Debug: Trying to spawn process with executable: %s\n", (char*)executable);

  /* We must first disable interrups and acquire the process_table lock. */
  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  /* Find empty spot in process_table. */
  for (i=0; i < PROCESS_MAX_PROCESSES; i++)
  {
    if (process_table[i].process_id == -1) break;
  }

  /* Check if the process_table is filled. */
  if (i == PROCESS_MAX_PROCESSES)
  {
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    return -1;
  }

  /* Since the process_table is not filled, then
     i contains the new process's process_id,
     which is also the index for in the process_table. */

  /* Copy the executable string into the process_control_block. */
  stringcopy(process_table[i].executable, executable, MAX_FILE_NAME);
  
  /* Set the state, process_id and retval. */
  process_table[i].process_state = RUNNING;
  process_table[i].process_id = i;
  process_table[i].retval = 0;
  process_table[i].parent_pid = -1;

  /* Enable interrups again, and release the process_table lock. */
  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  /* This is still parent process, so we must create new thread and let 
   * the spawned process run in that, and let that call process_start. */

  /* If i is 0, then we assume this is the initproc,
    and therefore should not run in a new thread. */
  if (i == 0)
  {
    intr_status = _interrupt_disable();
    spinlock_acquire(thread_get_slock());
    
    thread_get_current_thread_entry()->process_id = i;
    
    spinlock_release(thread_get_slock());
    _interrupt_set_state(intr_status);
    process_start(i);
  }

  /* Else create a new thread. */
  tid = thread_create(process_help_spawn, (uint32_t)i);

  /* tid is negative if no thread could be created.
     Which in that case we must cancel the process creation process.
     And mark the process_table slot to be empty and return with error. */
  if (tid < 0)
  {
    intr_status = _interrupt_disable();
    spinlock_acquire(&process_table_slock);

    process_mark_process_table_entry_empty(i);

    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    return -2;
  }

  /* Here is where this process should mark the new one as a child process.
     Not above, where we 'initialized' the process_table entry, because
     we shouldn't do it, if this is the first process, which will keep -1 as parent id. */
  
  process_table[i].parent_pid = process_get_current_process();

  /* Update the thread_block to contain its process_id. 
     Here we must disable interrupts again, and lock the
     thread_table spinlock, before updating it. */
  intr_status = _interrupt_disable();
  spinlock_acquire(thread_get_slock());

  thread_get_thread_entry(tid)->process_id = i;

  spinlock_release(thread_get_slock());
  _interrupt_set_state(intr_status);

  /* Mark the created thread to be ready to run. */
  thread_run(tid);

  /* Return the process id of the created process. */
  return i;
}

/* Stop a process and the thread running it.
   param retval, a user specified integer, which is passed on the
         parent process, if listening, see process_join.
 */
void process_finish(int retval) {
  interrupt_status_t intr_status;
  process_id_t pid;
  int i;

  /* Disable interrupts and lock the process_table spinlock,
     and the thread_table spinlock. */
  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);
  spinlock_acquire(thread_get_slock());

  /* Get the process id of this process. */
  pid = process_get_current_process();

  /* Here is where you should kill all your children. */
  for (i=0; i < PROCESS_MAX_PROCESSES; i++) {
    /* If process entry is child, kill it. */
    if (process_table[i].process_id != -1 &&
	process_table[i].parent_pid == pid) {
      if (process_table[i].process_state == ZOMBIE) {
	process_mark_process_table_entry_empty(i);
	DEBUG("process_Debug","Debug: process_finish with pid %d, removed dead child with pid %d\n",pid,i);
      } else {   
	process_table[i].parent_pid = -1;
	DEBUG("process_Debug","Debug: process_finish with pid %d, made child with pid %d parentless\n",pid,i);
      }
    }
  }

  if (process_table[pid].parent_pid == -1) {
    /* We are parent-less. Meaning parent process has already finished.*/
    process_mark_process_table_entry_empty(pid);
    DEBUG("process_Debug", "Debug: process_finish with pid %d had no parent\n", pid);
  } else {
    /* Set the state of this process to be ZOMBIE and set the return value. */
    process_table[pid].process_state = ZOMBIE;
    process_table[pid].retval = retval;
  }

  /* Enable interrupts again, and release the process_table spinlock,
     and the thread_table spinloc. */
  spinlock_release(thread_get_slock());
  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  DEBUG("process_Debug", "Debug: process_finish with pid %d is trying to wake one on %d\n", pid,pid);

  /* Wake up the sleeping parent, if any. */
  sleepq_wake((void*)pid);

  /* Disable interrupts and acquire the thread_table spinlock
     before using the table. */
  intr_status = _interrupt_disable();
  spinlock_acquire(thread_get_slock());

  vm_destroy_pagetable(thread_get_current_thread_entry()->pagetable);
  thread_get_current_thread_entry()->pagetable = NULL;

  spinlock_release(thread_get_slock());
  _interrupt_set_state(intr_status);

  /* 'Kill' this thread. */
  thread_finish();
}

/* Wait for a given child process to be finished.
   param pid, is the process id of the process you want to wait for.
   return retval, the return value specified by the given child process.
   return negative if error occoured.
          -1 if the given process id is out of range.
	  -2 if no process exist with the given process id.
	  -3 if the process id is yourself.
	  -4 if the process is not your child. */
int process_join(process_id_t pid) {
   interrupt_status_t intr_status;
   int retval;
   int ownPid;

   /* Se de to linier som fucker op.
   intr_status = _interrupt_disable();
  spinlock_acquire(thread_get_slock());
  int cpid = process_get_current_process();
  spinlock_release(thread_get_slock());
  _interrupt_set_state(intr_status); */

   DEBUG("process_Debug", "Debug: process_join trying to join pid: %d\n", pid);

   /* Make sure the given process is a valid process_id */
   if (pid < 0 || pid >= PROCESS_MAX_PROCESSES)
   {
     return -1;
   }

   /* Disable interrupts and lock the process_table spinlock. */
   intr_status = _interrupt_disable();
   spinlock_acquire(&process_table_slock);

   ownPid = process_get_current_process();

   /* See if the process exists. */
   if (process_table[pid].process_id == -1)
   {
     spinlock_release(&process_table_slock);
     _interrupt_set_state(intr_status);
     return -2;
   }

   /* Check if the given process id contains to yourself. */
   if (pid == ownPid)
   {
     spinlock_release(&process_table_slock);
     _interrupt_set_state(intr_status);
     return -3;
   }

   /* Here is where process_join should check if the process is a child process,
      of this process. If not then return -4. */
   if (ownPid != process_table[pid].parent_pid)
   {
     spinlock_release(&process_table_slock);
     _interrupt_set_state(intr_status);
     return -4;
   }

   /* Check if the process is already finished and therefore marked as a zombie,
      and if so don't go to sleep. */
   if (process_table[pid].process_state == ZOMBIE) goto isZombie;

   /* Uncomment -> fucker det op. Hvorfor? */
   /* process_table[cpid].process_state = WAITING;*/
   
   DEBUG("process_Debug","Debug: process_join putting process with pid %d to sleep on pid %d\n",ownPid,pid);
   /* Add this process to the sleep-queue. */
   sleepq_add((void*)pid);
   
   /* Release the process_table spinlock and enable interrupts. */
   spinlock_release(&process_table_slock);
   _interrupt_set_state(intr_status);

   /* Call thread_switch() manually like kernel/sleepq.c specifies.
      Really needed? Doesn't seems like it. */
   thread_switch();
   DEBUG("process_Debug","Debug: process_join, process with pid %d, woke up\n",ownPid);

   /* Disable interrupts and lock the process_table again. */
   intr_status = _interrupt_disable();
   spinlock_acquire(&process_table_slock);
   
   /* Uncomment -> fucker det op. Hvorfor? */
   /*   process_table[cpid].process_state = RUNNING;*/
   
 isZombie:
   /* Get the return value from the dead process. */
   retval = process_table[pid].retval;

   /* Mark the process_table entry to be empty. */
   process_mark_process_table_entry_empty(pid);
   
   /* Release the thread_table spinlock and enable interrups once again. */
   spinlock_release(&process_table_slock);
   _interrupt_set_state(intr_status);
   
   /* Return the return value from the dead process. */
   return retval;
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
