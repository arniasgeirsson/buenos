#include "tests/lib.h"

int main(void)
{
  int i;

  wrapper_writeString("Hello, I am validprog, I will count to ten!\n");
  
  for (i=1; i <= 10; i++)
  {
    wrapper_writeMlt("",i,"\n");
  }
  wrapper_writeString("That is it! Goodbye\n");
  syscall_exit(0);
  return 0;
}
