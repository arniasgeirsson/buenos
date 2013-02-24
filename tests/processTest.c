#include "tests/lib.h"

int main()
{
  syscall_write(1,(void*)"Made it so far!\n",16);
  syscall_exec("What?");
  syscall_halt();
  return 0;
}
