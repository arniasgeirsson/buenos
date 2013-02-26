#include "tests/lib.h"

/* Does nothing but returning with succes. */
int main(void)
{
  syscall_exit(0);
  return 0;
}
