#include "tests/lib.h"

int main(void)
{
  /* Trying to join init, ie. not own child */
  syscall_exit(syscall_join(0));
  return 0;
}
