#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int *p = f->esp;
  const void *buffer;
  unsigned int size;
  int i;
  switch(*p){
  case SYS_HALT:
      printf ("HALT\n");
      power_off();
      break;
  case SYS_CREATE:
      printf ("create system call!\n");
      const char *filename = *(p + 1);
      size = *(p + 2);
      f->eax = filesys_create(filename,size);
      break;
  case SYS_WRITE:
      printf ("write system call!\n");
      int *fileWriteDescriptor = *(p + 1);
      buffer = *(p + 2);
      size = *(p + 3);
      if(fileWriteDescriptor == STDOUT_FILENO){
      	putbuf(buffer,size);
        f->eax = size;
      }else{
        f->eax = file_write(fdOpen(fileWriteDescriptor),buffer,size);
      }
      break;
  case SYS_OPEN:
      printf ("open system call!\n");
      const char *fileToOpen = *(p + 1);
      struct file *fp = filesys_open(fileToOpen);
      f->eax = addFile(fp);
      break;
  case SYS_READ:
      printf ("read system call!\n");
      int *fileReadDescriptor = *(p + 1);
      buffer = *(p + 2);
      size = *(p + 3);
      if(fileReadDescriptor == STDIN_FILENO){
	for(i = 0; i < size; i++){
	  input_getc();
        }
      f->eax = size;
      }
      else{
	f->eax = file_read(fdOpen(fileReadDescriptor),buffer,size);
      }
      
      break;
  default:
      printf ("default system call! SYS_NR: ");
      printf ("%d \n",*p);
      break;
  }
}
