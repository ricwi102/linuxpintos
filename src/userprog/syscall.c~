#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
int addFile(struct file *f);
void removeFile(int fd);
struct file* fdOpen(int fd);


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
  unsigned int i;
  switch(*p){
  case SYS_HALT:
      printf ("HALT\n");
      power_off();
      break;
  case SYS_CREATE:
      printf ("create system call!\n");
      const char *filename = (const char*)(*(p + 1));
      size = *(p + 2);
      if(*filename != NULL){
        f->eax = filesys_create(filename,size);
      }else{
        f->eax = false;
      }
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
        struct file* openFile = fdOpen(fileWriteDescriptor);        
	if (openFile != NULL){
          f->eax = file_write(openFile,buffer,size);
        } else {
          f->eax = -1;
        }
      }
      break;
  case SYS_OPEN:
      printf ("open system call!\n");
      const char *fileToOpen = *(p + 1);     
      struct file* openFile = filesys_open(fileToOpen);
      if (openFile != NULL){
        f->eax = addFile(openFile);
      } else {
        f->eax = -1;
      }
      printf("%i", f->eax);
      printf("\n");
      break;
  case SYS_READ:
      printf ("read system call!\n");
      int *fileReadDescriptor = *(p + 1);      
      buffer = *(p + 2);
      size = *(p + 3);
      if(fileReadDescriptor == STDIN_FILENO){
	for(i = 0; i < size; i++){
          input_getc();
          //TODO: 
	  //*buffer = input_getc();
	  //buffer++;
        }
        f->eax = size;
      }
      else{ 
        struct file* openFile = fdOpen(fileReadDescriptor);  
        if (openFile != NULL){
          f->eax = file_read(openFile,buffer,size);
        } else {
          f->eax = -1;
        } 
      }
      
      break;
  case SYS_CLOSE:
      printf ("close system call!\n");
      int *fileCloseDescriptor = *(p + 1);
      file_close(fdOpen(fileCloseDescriptor));
      removeFile(fileCloseDescriptor);
      break;
  case SYS_EXIT:
      printf ("exit system call!\n");
      thread_exit();
      //TODO: free resources?
      break;
  default:
      printf ("default system call! SYS_NR: ");
      printf ("%d \n",*p);
      break;
  }
}



    int addFile(struct file *f){
      struct thread *t = thread_current();
      unsigned int i;
      for (i = 0; i < 128; ++i){
        if(t->fileArray[i] == NULL){
          t->fileArray[i] = f;
          return i+2;
        }
      }
        return -1;
    }

    void removeFile(int fd){
      struct thread *t = thread_current();
      t->fileArray[fd - 2] = NULL;
    }

    struct file* fdOpen(int fd){
      if (fd >= 0 && fd < 128){
        struct thread *t = thread_current();
        return t->fileArray[fd - 2];
      } else {
	return NULL;
      }
    }




















