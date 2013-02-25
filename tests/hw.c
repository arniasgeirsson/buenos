/*
 * Userland helloworld
 */

#include "tests/lib.h"

static const int BUFFER_SIZE = 20;

int main(void)
{
  char name[BUFFER_SIZE];
  int count;
  /* heap_init();*/
  /* puts("Hello, World!\n");*/
  syscall_write(1,"Hello World\n", 12);
  while (1) {
    
    /*    printf("Please enter your name (max %d chars): ", BUFFER_SIZE);*/
    syscall_write(1,"Please enter your name: ",25);
    count = syscall_read(0,name, BUFFER_SIZE);
    if(*(char*)name == '\0')
      break;
    name[count] = 0; /* Chomp off newline */
    /*
    printf("And hello to you, %s!\n", name);
    */
    syscall_write(1,"And hello to you!\n",19);
  }
  syscall_write(1,"Exit now!\n",10);
  /* puts("Now I shall exit!\n");*/
  syscall_exit(2);
  return 0;
}
