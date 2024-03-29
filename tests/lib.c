/*
 * Userland library functions
 *
 * Copyright (C) 2003 Juha Aatrokoski, Timo Lilja,
 *   Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: lib.c,v 1.6 2004/01/14 10:08:50 ttakanen Exp $
 *
 */

/* You probably want to add new functions to this file. To maintain
 * binary compatibility with other Buenoses (as probably required by
 * your assignments) DO NOT CHANGE EXISTING SYSCALL FUNCTIONS!
 */

#include "proc/syscall.h"
#include "tests/lib.h"


/* Halt the system (sync disks and power off). This function will
 * never return. 
 */
void syscall_halt(void)
{
  _syscall(SYSCALL_HALT, 0, 0, 0);
}


/* Load the file indicated by 'filename' as a new process and execute
 * it. Returns the process ID of the created process. Negative values
 * are errors.
 */
pid_t syscall_exec(const char *filename)
{
  return (int)_syscall(SYSCALL_EXEC, (uint32_t)filename, 0, 0);
}

/* Load the file indicated by 'filename' as a new process and execute
 * it. Returns the process ID of the created process. Negative values
 * are errors.
 */
pid_t syscall_execp(const char *filename, int argc, const char **argv)
{
  return (int)_syscall(SYSCALL_EXEC, (uint32_t)filename, 
                       (uint32_t) argc, 
                       (uint32_t) argv);
}


/* Exit the current process with exit code 'retval'. Note that
 * 'retval' must be non-negative since syscall_join's negative return
 * values are interpreted as errors in the join call itself. This
 * function will never return.
 */
void syscall_exit(int retval)
{
  _syscall(SYSCALL_EXIT, (uint32_t)retval, 0, 0);
}


/* Wait until the execution of the process identified by 'pid' is
 * finished. Returns the exit code of the joined process, or a
 * negative value on error.
 */
int syscall_join(pid_t pid)
{
  return (int)_syscall(SYSCALL_JOIN, (uint32_t)pid, 0, 0);
}


/* Create a new thread running in the same address space as the
 * caller. The thread is started at function 'func', and the thread
 * will end when 'func' returns. 'arg' is passed as an argument to
 * 'func'. Returns 0 on success or a negative value on error.
 */
int syscall_fork(void (*func)(int), int arg)
{
  return (int)_syscall(SYSCALL_FORK, (uint32_t)func, (uint32_t)arg, 0);
}


/* (De)allocate memory by trying to set the heap to end at the address
 * 'heap_end'. Returns the new end address of the heap, or NULL on
 * error. If 'heap_end' is NULL, the current heap end is returned.
 */
void *syscall_memlimit(void *heap_end)
{
  return (void*)_syscall(SYSCALL_MEMLIMIT, (uint32_t)heap_end, 0, 0);
}


/* Open the file identified by 'filename' for reading and
 * writing. Returns the file handle of the opened file (positive
 * value), or a negative value on error.
 */
int syscall_open(const char *filename)
{
  return (int)_syscall(SYSCALL_OPEN, (uint32_t)filename, 0, 0);
}


/* Close the open file identified by 'filehandle'. Zero will be returned
 * success, other values indicate errors. 
 */
int syscall_close(int filehandle)
{
  return (int)_syscall(SYSCALL_CLOSE, (uint32_t)filehandle, 0, 0);
}


/* Read 'length' bytes from the open file identified by 'filehandle'
 * into 'buffer', starting at the current file position. Returns the
 * number of bytes actually read (e.g. 0 if the file position is at
 * the end of file) or a negative value on error.
 */
int syscall_read(int filehandle, void *buffer, int length)
{
  return (int)_syscall(SYSCALL_READ, (uint32_t)filehandle,
                       (uint32_t)buffer, (uint32_t)length);
}


/* Set the file position of the open file identified by 'filehandle'
 * to 'offset'. Returns 0 on success or a negative value on error. 
 */
int syscall_seek(int filehandle, int offset)
{
  return (int)_syscall(SYSCALL_SEEK,
                       (uint32_t)filehandle, (uint32_t)offset, 0);
}


/* Write 'length' bytes from 'buffer' to the open file identified by
 * 'filehandle', starting from the current file position. Returns the
 * number of bytes actually written or a negative value on error.
 */
int syscall_write(int filehandle, const void *buffer, int length)
{
  return (int)_syscall(SYSCALL_WRITE, (uint32_t)filehandle, (uint32_t)buffer,
                       (uint32_t)length);
}


/* Create a file with the name 'filename' and initial size of
 * 'size'. Returns 0 on success and a negative value on error. 
 */
int syscall_create(const char *filename, int size)
{
  return (int)_syscall(SYSCALL_CREATE, (uint32_t)filename, (uint32_t)size, 0);
}

/* Create a file with the name 'filename' and initial size of
 * 'size'. Returns 0 on success and a negative value on error. 
 */
int syscall_delete(const char *filename)
{
  return (int)_syscall(SYSCALL_DELETE, (uint32_t)filename, 0, 0);
}

/* The following functions are not system calls, but convenient
   library functions inspired by POSIX and the C standard library. */

#ifdef PROVIDE_STRING_FUNCTIONS

/* Return the length of the string pointed to by s. */
size_t strlen(const char *s)
{
  size_t i;
  for (i=0; s[i]; i++);
  return i;
}

/* Copy all of src to after dest and return dest.  Make sure there is
   enough room before calling this function. */
char *strcpy(char *dest, const char *src)
{
  size_t i;
  for (i = 0; src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  // reached end, now src[i] == '\0'
  dest[i]='\0';
  return dest;
}

/* Copy as much of src as possible to after dest.  At most n
   characters from src will be copied. If there is no null byte among
   the first n bytes of src, the string placed in dest will not be
   null-terminated. */
char *strncpy(char *dest, const char *src, size_t n)
{
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  if (i < n) {
    dest[i] = '\0';
  }
  return dest;
}

/* Copy all of src to after dest and return dest.  Make sure there is
   enough room before calling this function. */
char *strcat(char *dest, const char *src)
{
  return strcpy(dest+strlen(dest), src);
}

char *strncat(char *dest, const char *src, size_t n)
{
  size_t dest_len = strlen(dest);
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[dest_len + i] = src[i];
  }
  dest[dest_len + i] = '\0';

  return dest;
}

int strcmp(const char *s1, const char *s2)
{
  return strncmp(s1, s2, 0x7fffffff);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  int i;
  for (i = 0; (s1[i] || s2[i]) && n > 0 ; i++, n--) {
    if (s1[i] < s2[i]) {
      return -1;
    } else if (s1[i] > s2[i]) {
      return 1;
    }
  }
  return 0;
}

int memcmp(const void* s1, const void* s2,size_t n)
{
  const unsigned char *p1 = s1, *p2 = s2;
  while(n--) {
    if( *p1 != *p2 ) {
      return *p1 - *p2;
    } else {
      p1++;
      p2++;
    }
  }
  return 0;
}

char *strstr(const char *s1, const char *s2)
{
  size_t n = strlen(s2);
  while(*s1)
    if(!memcmp(s1++,s2,n))
      return (char*)s1-1;
  return NULL;
}

void *memset(void *s, int c, size_t n) {
  byte *p = s;
  while (n-- > 0) {
    *(p++) = c;
  }
  return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
  byte *d = dest;
  const byte *s = src;
  while (n-- > 0) {
    *(d++) = *(s++);
  }
  return dest;
}

#endif

#ifdef PROVIDE_OWN_WRAPPERS

/* Helper for wrapper_writeInt, and convertIntToString. */
int intLen(int val)
{
  int len = 0;
  
  if (val < 0)
    val = val * -1;

  while (val > 0) {
    val = (val-(val%10)) / 10;
  }

  return len;
}

/* Convert a given integer val into a string
   and place it in str. */
void convertIntToString(int val, char *str)
{
  /* Somehow I can't use intLen(val) here.. */
  char r_str[12];
  int len = 0;
  int tmp = val;

  r_str[len] = '\0';

  int a;
  if (val == 0) {
    len++;
    r_str[len] = '0';
  } else if (val < 0) {
    tmp = tmp * -1;
  }
  while (tmp > 0) {
    len++;
    a = tmp % 10;
    r_str[len] = (char)a+48;
    tmp = (tmp-a) / 10;
  }

  if (val < 0) {
    len++;
    r_str[len] = '-';
  }

  int i = 0;
  while (len >= 0) {
    str[i] = r_str[len];
    len--;
    i++;
  }
}

/* Taken from FreeBSD. 
   Return the length of a string. */
int stringLength(char *str)
{
  const char *s;
  for (s = str; *s; ++s);
  return (s-str);
}

/* Write a string to the consol. */
void wrapper_writeString(char *str)
{
  syscall_write(1,str,stringLength(str));
}

/* Write an integer to the consol. */
void wrapper_writeInt(int val)
{
  char str[intLen(val)];
  convertIntToString(val,str);
  wrapper_writeString(str);
}

void wrapper_writeMlt(char *str, int val, char *str2)
{
  wrapper_writeString(str);
  wrapper_writeInt(val);
  wrapper_writeString(str2);
}

#endif

#ifdef PROVIDE_BASIC_IO

/* Write c to standard output.  Returns a non-negative integer on
   success. */
int putc1(char c)
{
  return syscall_write(stdout, &c, 1);
}

/* Write the string pointed to by s to standard output.  Returns a
   non-negative integer on success. */
int puts(const char* s)
{
  return syscall_write(stdout, s, strlen(s));
}

/* Read character from standard input, without echoing.  Returns a
   non-negative integer on success, which can be casted to char. */
int getc_raw(void)
{
  char c;
  syscall_read(stdin, &c, 1);
  return c;
}

/* Read character from standard input, with echoing.  Returns a
   non-negative integer on success, which can be casted to char. */
int getc(void)
{
  char c = getc_raw();
  syscall_write(stdout, &c, 1); /* Echo back at user. */
  return c;
}

/* Read up to size characters from standard input into the buffer s,
   with echoing.  Returns the number of characters read. */
ssize_t gets(char *s, size_t size)
{
  size_t count;
  for (count = 0; count+1 < size; s[count++] = getc());
  s[count+1] = '\0';
  return count;
}

/* Read up from standard input up to the first newline (\n) character,
   and at most size-1 characters, into the buffer s, with echoing and
   support for backspace.  Returns the number of characters read.  You
   can check whether a full line was read by seeing whether a newline
   precedes the terminating \0. */
ssize_t readline(char *s, size_t size)
{
  size_t count = 0;
  while (1) {
    int c = getc_raw();
    switch (c) {
    case '\r': /* Treat as newline */
    case '\n':
      putc1('\n');
      goto stop;
      break;
    case 127:
      if (count > 0) {
        putc1('\010');
        putc1(' ');
        putc1('\010');
        count--;
      }
      break;
    default:
      if (count<size-1) {
        putc1(s[count++]=c);
      }
    }
  }
 stop:
  s[count] = '\0';
  return count;
}

#endif

/* Formatted printing, from the lib/ directory of Buenos. */
#ifdef PROVIDE_FORMATTED_OUTPUT

#define FLAG_TTY     0x8000
#define FLAG_SMALLS  0x01
#define FLAG_ALT     0x02
#define FLAG_ZEROPAD 0x04
#define FLAG_LEFT    0x08
#define FLAG_SPACE   0x10
#define FLAG_SIGN    0x20


/* Output the given char either to the string or to the TTY. */
static void printc(char *buf, char c, int flags) {
  if (flags & FLAG_TTY) {
    /* do not output (terminating) zeros to TTY */
    if (c != '\0') putc1(c);
  } else
    *buf = c;
}


/* Output 'n' in base 'base' into buffer 'buf' or to TTY.  At least
 * 'prec' numbers are output, padding with zeros if needed, and at
 * least 'width' characters are output, padding with spaces on the
 * left if needed. 'flags' tells whether to use the buffer or TTY for
 * output and whether to use capital digits.
 */
static int print_uint(char *buf,
		      int size,
		      unsigned int n,
		      unsigned int base,
		      int flags,
		      int prec,
		      int width)
{
  static const char digits[32] = "0123456789ABCDEF0123456789abcdef";
  char rev[11]; /* space for 32-bit int in octal */
  int i = 0, written = 0;

  if (size <= 0) return 0;
  
  /* produce the number string in reverse order to the temp buffer 'rev' */
  do {
    if (flags & FLAG_SMALLS)
      rev[i] = digits[16 + n % base];
    else
      rev[i] = digits[n % base];
    i++;
    n /= base;
  } while (n != 0);

  /* limit precision and field with */
  prec = MIN(prec, 11);
  width = MIN(width, 11);

  /* zero pad until at least 'prec' digits written */
  while (i < prec) {
    rev[i] = '0';
    i++;
  }

  /* pad with spaces until at least 'width' chars written */
  while (i < width) {
    rev[i] = ' ';
    i++;
  }

  /* output the produced string in reverse order */
  i--;
  while (i >= 0 && written < size) {
    printc(buf++, rev[i], flags);
    written++;
    i--;
  }

  return written;
}


/* Scan a 10-base nonnegative integer from string 's'. The scanned
 * integer is returned, and '*next' is set to point to the string
 * immediately following the scanned integer.
 */
static int scan_int(const char *s, const char **next) {
  int value = 0;

  while (*s > '0' && *s < '9') {
    value = 10*value + (int)(*s - '0');
    s++;
  }

  if (next != NULL) *next = s;
  return value;
}

static int vxnprintf(char *buf,
		     int size,
		     const char *fmt,
		     va_list ap,
		     int flags)
{
  int written = 0, w, moremods;
  int width, prec;
  char ch, *s;
  unsigned int uarg;
  int arg;

  if (size <= 0) return 0;

  while (written < size) {
    ch = *fmt++;
    if (ch == '\0') break;

    /* normal character => just output it */
    if (ch != '%') {
      printc(buf++, ch, flags);
      written++;
      continue;
    }

    /* to get here, ch == '%' */
    ch = *fmt++;
    if (ch == '\0') break;

    flags &= FLAG_TTY; /*  preserve only the TTY flag */
    width = prec = -1;
    moremods = 1;

    /* read flags and modifiers (width+precision): */
    do {
      switch(ch) {
      case '#': /* alternative output */
        flags |= FLAG_ALT;
        break;

      case '0': /* zero padding */
        flags |= FLAG_ZEROPAD;
        break;

      case ' ': /* space in place of '-' */
        flags |= FLAG_SPACE;
        break;

      case '+': /* '+' in place of '-' */
        flags |= FLAG_SIGN;
        break;

      case '-': /* left align the field */
        flags |= FLAG_LEFT;
        break;

      case '.': /* value precision */
        prec = scan_int(fmt, &fmt);
        break;

      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9': /* field width */
        width = scan_int(fmt-1, &fmt);
        break;

      default: /* no more modifiers to scan */
        moremods = 0;
      }

      if (moremods) ch = *fmt++;
    } while (moremods && ch != '\0');


    if (ch == '\0') break;

    /* read the type of the argument : */
    switch(ch) {
    case 'i': /* signed integer */
    case 'd':
      arg = va_arg(ap, int);

      if (arg < 0) { /* negative value, print '-' and negate */
        printc(buf++, '-', flags);
        written++;
        arg = -arg;
      } if (flags & FLAG_SIGN) { /* '+' in place of '-' */
        printc(buf++, '+', flags);
        written++;
      } else if (flags & FLAG_SPACE) { /* ' ' in place of '-' */
        printc(buf++, ' ', flags);
        written++;
      }
      
      w = print_uint(buf, size-written, arg, 10, flags, 0, 0);
      buf += w;
      written += w;
      break;

    case 'o': /* octal integer */
      if (prec < width && (flags & FLAG_ZEROPAD)) prec = width;
      uarg = va_arg(ap, unsigned int);
      w = print_uint(buf, size-written, uarg, 8, flags, prec, width);
      buf += w;
      written += w;
      break;

    case 'u': /* unsigned integer */
      if (prec < width && (flags & FLAG_ZEROPAD)) prec = width;
      uarg = va_arg(ap, unsigned int);
      w = print_uint(buf, size-written, uarg, 10, flags, prec, width);
      buf += w;
      written += w;
      break;

    case 'p': /* memory pointer */
      flags |= FLAG_ALT;
    case 'x': /* hexadecimal integer, noncapitals */
      flags |= FLAG_SMALLS;
    case 'X': /* hexadecimal integer, capitals */

      if (flags & FLAG_ALT) { /* alt form begins with '0x' */
        printc(buf++, '0', flags);
        written++;
        if (written < size) {
          printc(buf++, 'x', flags);
          written++;
        }
        width -= 2;
      }
      if (prec < width && (flags & FLAG_ZEROPAD)) prec = width;

      uarg = va_arg(ap, unsigned int);
      w = print_uint(buf, size-written, uarg, 16, flags, prec, width);
      buf += w;
      written += w;
      break;

    case 'c': /* character */
      arg = va_arg(ap, int);
      printc(buf++, (char)arg, flags);
      written++;
      break;

    case 's': /* string */
      s = va_arg(ap, char*);
      w = size;
      if (prec != -1 && written+prec < size) w = written+prec;
      while (written < w && *s != '\0') {
        printc(buf++, *s++, flags);
        written++;
      }
      break;

    default: /* unknown type, just output */
      printc(buf++, ch, flags);
      written++;
    }
  }
  /* the string was truncated */
  if (written == size) { 
    buf--;
    written = -1;
  }
  printc(buf, '\0', flags); /* terminating zero */

  return written;
}

int printf(const char *fmt, ...) {
  va_list ap;
  int written;

  va_start(ap, fmt);
  written = vxnprintf((char*)0, 0x7fffffff, fmt, ap, FLAG_TTY);
  va_end(ap);

  return written;
}

int snprintf(char *str, int size, const  char  *fmt, ...) {
  va_list ap;
  int written;

  va_start(ap, fmt);
  written = vxnprintf(str, size, fmt, ap, 0);
  va_end(ap);
  return written;
}

#endif

/* Heap allocation. */
#ifdef PROVIDE_HEAP_ALLOCATOR

typedef struct free_block {
  size_t size;
  struct free_block *next;
} free_block_t;

static const size_t MIN_ALLOC_SIZE = sizeof(free_block_t);

free_block_t *free_list = NULL;

byte heap[HEAP_SIZE]; /* obselete */
void *heap_start = NULL; /* Start address of the heap. */
void *heap_end = NULL; /* End address of the heap. */

/* Initialise the heap - malloc et al won't work unless this is called
   first. */
/* heap_init should now be obselete. */
void heap_init()
{
  free_list = (free_block_t*) heap;
  free_list->size = HEAP_SIZE;
  free_list->next = NULL;
}

#define MAX_MALL_PTRS 64
uint32_t mallptrs[MAX_MALL_PTRS];
int mallptrs_size;

/* Initialize the mallptrs array by setting
   all entries to 0. */
void mallptrs_init()
{
  int i;
  mallptrs_size = 0;
  for (i=0; i < MAX_MALL_PTRS; i++) {
    mallptrs[i] = 0;
  }
}

/* Insert the adress of a pointer in
   mallptrs. */
void *mallptrs_insert(void *ptr)
{
  int i;
  for (i=0; i < MAX_MALL_PTRS; i++) {
    if (mallptrs[i] == 0) {
      mallptrs[i] = (uint32_t)ptr;
      mallptrs_size++;
      return ptr;
    }
  }
  printf("mallptrs_insert: array is filled%s\n","");
  return NULL;
}

/* Check if the adress of a pointer exists
   in mallptrs. */
int mallptrs_member(void *ptr)
{
  int i;
  for (i=0; i < MAX_MALL_PTRS; i++) {
    if (mallptrs[i] == (uint32_t)ptr) {
      return 1;
    }
  }
  return 0;
}

/* Remove the adress of a pointer from
   mallptrs. */
void mallptrs_remove(void *ptr)
{
  int i;
  for (i=0; i < MAX_MALL_PTRS; i++) {
    if (mallptrs[i] == (uint32_t)ptr) {
      mallptrs[i] = 0;
      mallptrs_size--;
      return;
    }
  }
  printf("mallptrs_remove: pointer was not in mallptrs%s\n","");
  return;
}

/* Return a block of at least size bytes, or NULL if no such block 
   can be found.  */
void *malloc(size_t size) {
  free_block_t *block;
  free_block_t **prev_p; /* Previous link so we can remove an element */
  void *old_end, *new_end;
  free_block_t *new_block, *tmp;  

  if (size == 0) {
    return NULL;
  }

  if (mallptrs_size >= MAX_MALL_PTRS) {
    printf("Malloc: mallptrs array is filled up%s\n","");
    return NULL;
  }

  /* Ensure block is big enough for bookkeeping. */
  size=MAX(MIN_ALLOC_SIZE,size);
  /* Word-align */
  if (size % 4 != 0) {
    size &= ~3;
    size += 4;
  }

  /* Iterate through list of free blocks, using the first that is
     big enough for the request. */
  for (block = free_list, prev_p = &free_list;
       block;
       prev_p = &(block->next), block = block->next) {
    printf("Malloc: Free-list block: pointer %d, size %d, next %d\n"
	   ,(uint32_t)block,block->size,(block->next != NULL));
    if ( (int)( block->size - size - sizeof(size_t) ) >= 
         (int)( MIN_ALLOC_SIZE+sizeof(size_t) ) ) {
      /* Block is too big, but can be split. */
      block->size -= size+sizeof(size_t);
      free_block_t *new_block =
        (free_block_t*)(((byte*)block)+block->size);
      new_block->size = size+sizeof(size_t);
      /*return ((byte*)new_block)+sizeof(size_t);*/
      return mallptrs_insert(((byte*)new_block)+sizeof(size_t));
    } else if (block->size >= size + sizeof(size_t)) {
      /* Block is big enough, but not so big that we can split
         it, so just return it */
      *prev_p = block->next;
      /*return ((byte*)block)+sizeof(size_t);*/
      return mallptrs_insert(((byte*)block)+sizeof(size_t));
    }
    /* Else, check the next block. */
  }

  /* If no suitable free block was found, we must
     allocate a new one. */

  /* If this is the first time, we get the addresse of
     our process's heap end, for bookkeeping.
     And initialize the mallptrs array. */
  if (heap_start == NULL) {
    heap_start = syscall_memlimit(NULL);
    heap_end = heap_start;
    mallptrs_init();
    printf("Malloc: Heap_start: %d\n", (uint32_t)heap_start);
  }

  printf("Malloc: Size: %d, heapend: %d\n", size, (uint32_t)heap_end); 

  old_end = heap_end;
  heap_end = (void*)((uint32_t)heap_end + size+ sizeof(size_t));
  /* Try and allocate more memory */
  new_end = syscall_memlimit(heap_end);

  /* Check if it was possible to get more memory. */
  if(new_end == NULL){
    /* No heap space left. */
    printf("Malloc: syscall_memlimit returned NULL%s\n","");
    heap_end = (void*)((uint32_t)heap_end - size+ sizeof(size_t));
    return NULL;
  }

  printf("Malloc: memlimit returned %d\n", (uint32_t)new_end);

  /* Make sure that the new end is not lower than what
     the user requested. */
  if(new_end < heap_end) {
    printf("Malloc: new_end (%d) was lower than what was requested (%d)\n"
	   ,(uint32_t)new_end,(uint32_t)heap_end);
    return NULL;
  }

  /* Make sure the new end is not lower than the old.
     It should not happen. */
  if(old_end >= new_end) {
    printf("Malloc: new_end was lower or equal to the old_end%s\n","");
    heap_end = (void*)((uint32_t)heap_end - size+ sizeof(size_t));
    return NULL;
  }

  /* Create a new free block with the size we just allocated. */
  new_block = (free_block_t*)((byte*)old_end);
  new_block->size = new_end - old_end;
  new_block->next = NULL;
  printf("Malloc: Created free block with size %d and address %d\n",new_block->size, (uint32_t)new_block);

  /* Attach it too the list of free blocks in a 
     sorted manor. That is at the end. */
  if (free_list == NULL) {
    printf("Malloc: free_list is null%s\n","");
    free_list = new_block;
  } else {
    printf("Malloc: free_list is not null%s\n","");
    tmp = free_list;
    
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }

    tmp->next = new_block;
    printf("Malloc: out of loop%s\n","");
  }

  /* Update the heap end to the new end address. */
  heap_end = new_end;
  
  /* Recursively call malloc which should now find the free block
     we just created and fullfill the user request for more memory. */
  return malloc(size);
}

/* Return the block pointed to by ptr to the free pool. */
void free(void *ptr)
{

  /* Make sure that the pointer is within the space of
     the already allocated memory space. */
  if (ptr < heap_start || ptr > heap_end) {
    printf("Error: free: ptr (%d) given to free is either below heap_start (%d) or above heap_end (%d)\n"
	   ,(uint32_t)ptr,(uint32_t)heap_start,(uint32_t)heap_end);
    return;
  }
  
  /* Make sure that the pointer ptr was previously allocated by malloc. */
  if (!mallptrs_member(ptr)) {
    printf("Error: free: pointer %d was not previous allocated by malloc!\n",(uint32_t)ptr);
    return;
  }
  /* Remove the ptr from the list of allocated pointers. */
  mallptrs_remove(ptr);

  if (ptr != NULL) { /* Freeing NULL is a no-op */
    free_block_t *block = (free_block_t*)((byte*)ptr-sizeof(size_t));
    free_block_t *cur_block;
    free_block_t *prev_block;

    /* Iterate through the free list, which is sorted by
       increasing address, and insert the newly freed block at the
       proper position. */
    for (cur_block = free_list, prev_block = NULL; 
         ;
         prev_block = cur_block, cur_block = cur_block->next) {
      if (cur_block > block || cur_block == NULL) {
        /* Insert block here. */
        if (prev_block == NULL) {
          free_list = block;
        } else {
          prev_block->next = block;
        }
        block->next = cur_block;

        if (prev_block != NULL &&
            (size_t)((byte*)block - (byte*)prev_block) == prev_block->size) {
          /* Merge with previous. */
          prev_block->size += block->size;
          prev_block->next = cur_block;
          block = prev_block;
        }

        if (cur_block != NULL &&
            (size_t)((byte*)cur_block - (byte*)block) == block->size) {
          /* Merge with next. */
          block->size += cur_block->size;
          block->next = cur_block->next;
        }
        return;
      }
    }
  }
}

void *calloc(size_t nmemb, size_t size)
{
  size_t i;
  byte *ptr = malloc(nmemb*size);
  if (ptr != NULL) {
    for (i = 0; i < nmemb*size; i++) {
      ptr[i] = 0;
    }
  }
  return ptr;
}

void *realloc(void *ptr, size_t size)
{
  byte *new_ptr;
  size_t i;
  if (ptr == NULL) {
    return malloc(size);
  }

  if (ptr != NULL && size == 0) {
    free(ptr);
    return NULL;
  }

  /* Simple implementation: allocate new space and copy the contents
     over.  Exercise: Improve this by searching through the free
     list and seeing whether an actual enlargement is possible. */
  new_ptr = malloc(size);
  if (new_ptr != NULL) {
    for (i = 0; i < size; i++) {
      new_ptr[i] = ((byte*)ptr)[i];
    }
    free(ptr);
  }
  return new_ptr;
}

#endif

#ifdef PROVIDE_MISC

int atoi(const char *nptr)
{
  int i;
  int retval = 0;
  int n = strlen(nptr);
  for (i = 0; i < n; i++) {
    if (nptr[i] < '0' || nptr[i] > '9') {
      break;
    }
    retval = retval * 10 + (nptr[i] - '0');
  }
  return retval;
}

#endif
