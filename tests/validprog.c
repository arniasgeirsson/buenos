#include "tests/lib.h"

static const char validprog[] = "[disk1]hw";

int main(void)
{
  int i;
  char str[3];

  syscall_write(1, "Hello, I am validprog, I will count to ten!\n",44);
  str[1] = '\n';
  str[2] = '\0';
  
  syscall_join(syscall_exec(validprog));
  for (i=0; i <= 10; i++)
  {
    str[0] = (char)i+36;
    syscall_write(1,str,2);
  }
  syscall_write(1,"That is it! Goodbye\n",20);
  syscall_exit(4);
  return 0;
}
