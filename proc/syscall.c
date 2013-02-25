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

int syscall_write(int fhandle, const void *buffer, int length){
  device_t *dev;
  gcd_t *gcd;
  int bytes = 0;
  int i;

  dev = device_get(YAMS_TYPECODE_TTY, fhandle - fhandle);
  if(dev == NULL)
    return -1;

  gcd = (gcd_t *)dev->generic_device;
  if(gcd == NULL)
    return -1;

  for(i = 0; i < length; i++){
    if(*(char*)(buffer + i) == '\0')      
      break;
    bytes += gcd->write(gcd, buffer + i, 1);
  }
  return bytes;
}

int syscall_read(int fhandle, void *buffer, int length){
  device_t *dev;
  gcd_t *gcd;
  int i;
  int bytes = 0;

  dev = device_get(YAMS_TYPECODE_TTY, fhandle);
  if(dev == NULL)
    return -1;

  gcd = (gcd_t *)dev->generic_device;
  if(gcd == NULL)
    return -1;

  gcd->write(gcd, (void*)"> ", 2);

  for(i = 0; i < length-1; i++){
    bytes += gcd->read(gcd, buffer + i, length);
    
    if(*(char*)(buffer + i) == '\r')      
      break;

    gcd->write(gcd, buffer + i, 1);
  }

  ((char*)buffer)[i] = '\0';
   gcd->write(gcd, (void*)"\n", 1);

  return bytes;
}

int syscall_exec(const char *filename)
{
  DEBUG("process_Debug","Sycall_exec called with filename: %s\n", (char*)filename);
  int npid = process_spawn(filename);
  DEBUG("process_Debug", "syscall_exec: new pid = %d\n",npid);
  return npid;
}

void syscall_exit(int retval)
{
  DEBUG("process_Debug","syscall_exit: retval = %d\n", retval);
  retval = retval; /* How to update retval? See hw.c from buenos-ex2. */
  process_finish(retval);
}

int syscall_join(int pid)
{
  DEBUG("process_Debug", "syscall_join: pid is = %d\n",pid);
  int pidjoin = process_join(pid);
  DEBUG("process_Debug","syscall_join: pidjoin = %d\n",pidjoin);
  return pidjoin;
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
      syscall_write(FILEHANDLE_STDOUT, 
		    (void*)user_context->cpu_regs[MIPS_REGISTER_A2],
		    (int)user_context->cpu_regs[MIPS_REGISTER_A3]);
      break;
    case SYSCALL_READ:
      syscall_read(FILEHANDLE_STDIN, 
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
      DEBUG("process_Debug", "syscall join!!!\n");
      int a1 = (int)user_context->cpu_regs[MIPS_REGISTER_A1];
      int a2 = (int)user_context->cpu_regs[MIPS_REGISTER_A2];
      int a3 = (int)user_context->cpu_regs[MIPS_REGISTER_A3];
      DEBUG("process_Debug", "a1 = %d, a2 = %d, a3 = %d\n",a1,a2,a3);
      user_context->cpu_regs[MIPS_REGISTER_V0] = syscall_join((int)user_context->cpu_regs[MIPS_REGISTER_A1]);
      break;
    default: 
      KERNEL_PANIC("Unhandled system call\n");
    }

    /* Move to next instruction after system call */
    user_context->pc += 4;
}
