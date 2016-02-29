#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
int addFile(struct file *f);
void removeFile(int fd);
struct file* fdOpen(int fd);
bool check_ptr(char* ptr);
bool check_mult_ptrs(void* ptr, int args);


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
	if (check_ptr(p)){
		f->eax = -1;
		sys_exit(-1);
	} else {
		size_t size;
		unsigned int i;
		switch(*p){
		case SYS_HALT:
		    power_off();
		    break;
		case SYS_CREATE:{
				if (check_ptr(p + 2)){
						f->eax = false;
						sys_exit(-1);
				}

		    const char *filename = (const char*)(*(p + 1));				
		    size = *(p + 2);
		    if(filename != NULL && size >= 0){
		      f->eax = filesys_create(filename,size);
		    }else{					
		      f->eax = false;
					sys_exit(-1);					
		    }
		    break; }
		case SYS_OPEN:{
				if (check_mult_ptrs(p, 1)){
						f->eax = -1;
						sys_exit(-1);
				}

		    const char *fileToOpen = (const char*)*(p + 1);     
		    struct file* openFile = (struct file*)filesys_open(fileToOpen);
		    if (openFile != NULL){
		      f->eax = addFile(openFile);
		    } else {        
		      f->eax = -1;
		    }
		    break; }
		case SYS_CLOSE:{
				if (check_mult_ptrs(p, 1)){
						f->eax = -1;
						sys_exit(-1);
				}

		    int fileCloseDescriptor = *(p + 1);
		    file_close(fdOpen(fileCloseDescriptor));
		    removeFile(fileCloseDescriptor);
		    break; }
		case SYS_READ:{
				if (check_mult_ptrs(p, 3)){
						f->eax = -1;
						sys_exit(-1);
				}

		    int fileReadDescriptor = *(p + 1);      
		    buffer = (const void*)(*(p + 2));
				size = *(p + 3);		
		    
		    if(fileReadDescriptor == STDIN_FILENO){
					for(i = 0; i < size; i++){
						if (check_ptr((char*)buffer + i)){
							f->eax = -1;
							sys_exit(-1);
						}					
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
				if (check_mult_ptrs(p, 3)){
						f->eax = false;
						sys_exit(-1);
				}

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
					if (openFile != NULL){
		      	f->eax = file_write(openFile,buffer,size);
		    	} else {
		    		f->eax = -1;
		    	}
		    }
		    break; }
		case SYS_EXIT:{
				if (check_mult_ptrs(p, 2)){
						f->eax = false;
						sys_exit(-1);
				}

				int exit_status = (int)(*(p + 1));
				f->eax = exit_status;
				sys_exit(exit_status);
		    break; }
		case SYS_EXEC:{
				if (check_mult_ptrs(p, 1)){
						f->eax = -1;
						sys_exit(-1);
				}

				char *filename = (const char*)(*(p + 1));
				int pid = process_execute(filename);
				if (pid == TID_ERROR) { f->eax =  -1; }
				else f->eax = pid;				
				break; }
		case SYS_WAIT:{
				if (check_mult_ptrs(p, 1)){
						f->eax = -1;
						sys_exit(-1);
				}

				int exit_value = process_wait((tid_t) (*(p + 1)));
				f->eax = exit_value;
				break; }
		default:{
		    printf ("default system call! SYS_NR: ");
		    printf ("%d \n",*p);
		    break; }
		}
  }
}

		void check_ptr(char* ptr){
			if (ptr >= PHYS_BASE || (pagedir_get_page(thread_current()->pagedir, ptr) == NULL)){
				sys_exit(-1);
			}
		}
	
		bool check_mult_ptrs(void* ptr, int args){
			if ((ptr + args) >= PHYS_BASE) {return true;}

			uint8_t i;
			for (i = 1; i <= args; ++i){
				if (check_ptr(*(char*)(ptr + i))) {return true;}
			}
			return false;
		}



		void sys_exit(int exit_status){
			printf("%s: exit(%d)\n", thread_current()->name, exit_status);		
			thread_current()->cs->exit_status = exit_status;
			thread_exit();			
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
		  file_close(f);
        return -1;
    }

    void removeFile(int fd){
			if (fd >= 2 && fd < 130){
      	struct thread *t = thread_current();
      	t->fileArray[fd - 2] = NULL;
			}
    }

    struct file* fdOpen(int fd){
      if (fd >= 2 && fd < 130){
        struct thread *t = thread_current();				
        return t->fileArray[fd - 2];
      } else {
			return NULL;
      }
    }
   














