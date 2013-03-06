#include "tests/lib.h"

int main(void) 
{
  wrapper_writeString("Started!!\n");
  /*
  int *h;
  int g = 123;
  h = &g;
  h = h +2121231321;
  wrapper_writeString("About to do it111!!\n");

  int j = *h;
  wrapper_writeString("About to do it222!!\n");
  j = j;
  wrapper_writeString("About to do it333!!\n");


  wrapper_writeMlt("J is ",j,"\n");*/
  syscall_halt();
  return 0;
}
