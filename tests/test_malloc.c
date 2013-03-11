#include "tests/lib.h"

int main (void){
  void *a,*b,*c,*d,*e,*f;

  /*
    malloc:
    - malloc med 0 bytes
    - malloc med 10 bytes
    - malloc med 1000 bytes
    - malloc med -10
    - malloc int[4] and place 3 ints, show they are correct,
             (place one more show it goes wrong?)
	     free it, should be no more.(? first when it is overwritten?)
    - malloc indtil der ikke er flere pages


    free:
    - free pointer til 0
    - free pointer til 10
    - free pointer til 1000
    - free pointer til 1000 igen
    - free pointer som ikke kommer fra malloc inden for grænsen
    - free pointer som ikke kommer fra malloc uden for grænsen
    - free pointer som ikke kommer fra malloc men som har samme værdi som en der gør

   */

  /* Malloc test. */
  /* Malloc some simple values. */
  printf("---------- 1.0 ----------%s\n","");
  a = malloc(0); /* Should not fault, simply return NULL. */
  printf("Malloc test: a is NULL = %d\n",a==NULL);

  printf("---------- 2.0 ----------%s\n","");
  b = malloc(10); /* Should result in some adress. */

  printf("---------- 3.0 ----------%s\n","");
  c = malloc(1000); /* Should result in some adress. */

  printf("---------- 4.0 ----------%s\n","");
  /* d = malloc(-10);*/ /* Should result in kernel panic, which is also does. */
  d = NULL;
  printf("---------- 5.0 ----------%s\n","");

  e = NULL;/*malloc(10);*/ /* Results in a unexpected error. */ 
  printf("---------- 6.0 ----------%s\n","");

  /* Free test. */
  /* Free some of the simple values from above. */
  free(a);
  printf("---------- 7.0 ----------%s\n","");
  free(b);
  printf("---------- 8.0 ----------%s\n","");
  free(c);
  printf("---------- 9.0 ----------%s\n","");
  free(c);
  printf("---------- 10.0 ----------%s\n","");
  free(d);
  printf("---------- 11.0 ----------%s\n","");
  free(e);
  printf("---------- 12.0 ----------%s\n","");

  /* Test some of the boundary cases. */
  f = (void*)22000;
  free(f);
  printf("---------- 13.0 ----------%s\n","");
  f = (void*)1;
  free(f);
  printf("---------- 14.0 ----------%s\n","");
  a = malloc(10);
  f = a;
  free(f);
  printf("---------- 15.0 ----------%s\n","");

  syscall_halt();
  return 0;
}
