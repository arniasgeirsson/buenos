#include "tests/lib.h"

static const char validprog[] = "[disk1]validprogram2";

int main(void)
{
  int i;
  char str[3];

  syscall_write(1, "Validprogram1: Caling 'vallidprogram2'\n",39);
  
  syscall_exec(syscall_exec(validprog));

  syscall_write(1,"That is it! Goodbye\n",20);
  syscall_exit(4);
  return 0;
}
