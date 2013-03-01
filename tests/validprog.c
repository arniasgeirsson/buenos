#include "tests/lib.h"

/* Writes 10 integers to the console, then calls exit with succes. */
int main(void)
{
  int i;

  wrapper_writeString("Hello, I am validprog, I will count to ten!\n");
  
  for (i=1; i < 11; i++)
  {
    wrapper_writeMlt("",i,"\n");
  }
  wrapper_writeString("That is it! Goodbye\n");
  syscall_exit(0);
  return 0;
}
