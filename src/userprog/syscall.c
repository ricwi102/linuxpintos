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
  size_t size;
  unsigned int i;
  switch(*p){
  case SYS_HALT:
      power_off();
      break;
  case SYS_CREATE:{
      const char *filename = (const char*)(*(p + 1));
      size = *(p + 2);
      if(*filename != NULL){
        f->eax = filesys_create(filename,size);
      }else{
        f->eax = false;
      }
      break; }
  case SYS_OPEN:{
      const char *fileToOpen = (const char*)*(p + 1);     
      struct file* openFile = (struct file*)filesys_open(fileToOpen);
      if (openFile != NULL){
        f->eax = addFile(openFile);
      } else {
        f->eax = -1;
      }
      break; }
  case SYS_CLOSE:{
      int fileCloseDescriptor = *(p + 1);
      file_close(fdOpen(fileCloseDescriptor));
      removeFile(fileCloseDescriptor);
      break; }
  case SYS_READ:{
      int fileReadDescriptor = *(p + 1);      
      buffer = (const void*)(*(p + 2));
      size = *(p + 3);
      if(fileReadDescriptor == STDIN_FILENO){
	for(i = 0; i < size; i++){
	  *((char*)buffer + i) = input_getc();
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
      break; }
  case SYS_WRITE:{
      int fileWriteDescriptor = *(p + 1);
      buffer = (const void*)(*(p + 2));
      size = *(p + 3);
      if(fileWriteDescriptor == STDOUT_FILENO){
	static const size_t chunk_size_max = 200;
	for (i = 0; i <= (size/chunk_size_max); ++i){
	  size_t chunk_size = (size / chunk_size_max) > i ? chunk_size_max : (size % chunk_size_max);

      	  putbuf((buffer + i*chunk_size_max), chunk_size);          
	}
	f->eax = size;
      }else{   
        struct file* openFile = fdOpen(fileWriteDescriptor);        
	if (openFile != NULL /*&& openFile->deny_write == false*/){
          f->eax = file_write(openFile,buffer,size);
        } else {
          f->eax = -1;
        }
      }
      break; }
  case SYS_EXIT:{
      thread_exit();
      //Freed the file-array in thread_exit()
      break; }
  default:{
      printf ("default system call! SYS_NR: ");
      printf ("%d \n",*p);
      break; }
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
      if (fd >= 2 && fd < 130){
        struct thread *t = thread_current();				
        return t->fileArray[fd - 2];
      } else {
	return NULL;
      }
    }
   














