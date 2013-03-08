#include "tests/lib.h"

int main (void){
  wrapper_writeString("----- 0.5 -----\n");
  void* a = malloc(10);
  wrapper_writeString("----- 0.6 -----\n");
  if(a == NULL)
    wrapper_writeString("1 FAIL\n");
  wrapper_writeString("----- 1 -----\n");
  void* b = malloc(100);
  if(b == NULL)
    wrapper_writeString("2 FAIL\n");
  wrapper_writeString("----- 2 -----\n");
  wrapper_writeString("----- 3 -----\n");
  free(a);

  wrapper_writeString("----- 4 -----\n");
  free(b);

  free(b+1000);

  syscall_halt();
  return 1; 
  
}
