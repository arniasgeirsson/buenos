#include "tests/lib.h"

int main(void)
{
  int i;
  char str[3];

  syscall_write(1, "Hello, I am validprog, I will count to nine!\n",45);
  str[1] = '\n';
  str[2] = '\0';
  
  for (i=0; i < 10; i++)
  {
    str[0] = (char)i+48;
    syscall_write(1,str,2);
  }
  syscall_write(1,"That is it! Goodbye\n",20);
  syscall_exit(4);
  return 0;
}
