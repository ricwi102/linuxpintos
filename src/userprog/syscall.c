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
bool valid_ptr(void* ptr);
bool check_mult_ptr(void* ptr, int args);
bool valid_buffer(void* ptr, size_t amount);


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
	if (valid_ptr(p)){		
		size_t size;
		unsigned int i;
		switch(*p){
		case SYS_HALT:{
		    power_off();
		    break;	}
		case SYS_CREATE:{
				if (check_mult_ptr(p, 2) && valid_buffer(*(p + 1), strlen(*(p + 1)) )){
				  const char *filename = (const char*)(*(p + 1));				
				  size = *(p + 2);
				  if(filename != NULL && size >= 0){
				    f->eax = filesys_create(filename,size);
				  }else{					
				    f->eax = false;
						sys_exit(-1);					
				  }
				}
		    break; }
		case SYS_OPEN:{
				if (check_mult_ptr(p, 1) && valid_buffer(*(p + 1), strlen(*(p + 1)) )){
					const char *fileToOpen = (const char*)*(p + 1);     
				  struct file* openFile = (struct file*)filesys_open(fileToOpen);
				  if (openFile != NULL){
				    f->eax = addFile(openFile);
				  }else{        
				    f->eax = -1;
				  }
				}
		    break; }
		case SYS_CLOSE:{
				if (check_mult_ptr(p, 1)){
					int fileCloseDescriptor = *(p + 1);
				  file_close(fdOpen(fileCloseDescriptor));
				  removeFile(fileCloseDescriptor);
				}
		    break; }
		case SYS_READ:{
				if (check_mult_ptr(p, 3) && valid_buffer(*(p + 2), (size_t)*(p + 3))){				
				  int fileReadDescriptor = *(p + 1);      
				  buffer = (const void*)(*(p + 2));
					size = *(p + 3);		
				  
				  if(fileReadDescriptor == STDIN_FILENO){
						for(i = 0; i < size; i++){
							if (valid_ptr((char*)buffer + i)){
								*((char*)buffer + i) = input_getc();
							}								
						}
						f->eax = size;
					}else if(fileReadDescriptor == STDOUT_FILENO){
						f->eax = -1;
					}else{ 
				    struct file* openFile = fdOpen(fileReadDescriptor);  
				    if (openFile != NULL){
				      f->eax = file_read(openFile,buffer,size);
				    } else {
				      f->eax = -1;
				    } 
				  }
				}
		    break; }
		case SYS_WRITE:{
				if (check_mult_ptr(p, 3) && valid_buffer(*(p + 2), (size_t)*(p + 3)) ){
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
				  }else if(fileWriteDescriptor == STDIN_FILENO){
						f->eax = -1;
					}else{   
				 		struct file* openFile = fdOpen(fileWriteDescriptor);        
						if (openFile != NULL){
				    	f->eax = file_write(openFile,buffer,size);
				  	} else {
				  		f->eax = -1;
				  	}
					}
		    }
		    break; }
		case SYS_EXIT:{
				if (check_mult_ptr(p, 1)){
					int exit_status = (int)(*(p + 1));
					f->eax = exit_status;
					sys_exit(exit_status);
				}
		    break; }
		case SYS_EXEC:{
				printf("TEST1 \n");
				if (check_mult_ptr(p, 1) && valid_buffer(*(p + 1), strlen((char*)*(p + 1)) ) ){
					printf("TEST2 \n");
					char *filename = (const char*)(*(p + 1));
					int pid = process_execute(filename);
					if (pid == TID_ERROR) { 
						f->eax =  -1;					
					}
					else f->eax = pid;		
				}		
				break; }
		case SYS_WAIT:{
				if (check_mult_ptr(p, 1)){
					int exit_value = process_wait((tid_t) (*(p + 1)));
					f->eax = exit_value;
				}
				break; }
		default:{
		    printf ("default system call! SYS_NR: ");
		    printf ("%d \n",*p);
		    break; }
		}
  }
}


		/* Checks if a pointer is valid for that thread and exits if it is not.
		 	 Valid -> Smaller than PHYS_BASE and within one of the threads pages.  
		 	 Checks if the pointer is atleast on an adress 4 smaller than phys-base, 
			 since a pointer is always 4 bytes.*/
		bool valid_ptr(void* ptr){ 
			printf("TEST1 validptr \n");
			if (ptr >= (PHYS_BASE - 4) || (pagedir_get_page(thread_current()->pagedir, ptr + 3) == NULL)){
				printf("TESTfalse validptr \n");
				sys_exit(-1);
				return false;
			}
			printf("TESTtrue validptr \n");
			return true;
		}
	

		/* Checks multiple pointers with (valid_ptr(void' ptr)) starting from the
			 pointer after th given pointer and checks (args) pointers afterwrds. */
		bool check_mult_ptr(void* ptr, int args){
			printf("TEST1 multptr \n");
			uint8_t i;
			for (i = 1; i <= args; ++i){
				printf("TESTloop multptr \n");
				if (!valid_ptr(ptr + i)) {return false;}
			}
			return true;
		}

		bool valid_buffer(void* ptr, size_t amount){		
			printf("TEST1 validbuf amount = %i \n", amount);
			size_t i;
			for (i = 0; i < amount; ++i){
				printf("TESTloop validbuf \n");
				if (!valid_ptr((ptr + i) )) {return false;}
			}
			return true;
		}


		/* exits and closes the thread with the given exit_status */
		void sys_exit(int exit_status){
			printf("%s: exit(%d)\n", thread_current()->name, exit_status);		
			thread_current()->cs->exit_status = exit_status;
			thread_exit();			
		}

		/* Adds a file to the file array if it isn't full */
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

		/* Sets the position of the file descriptor to NULL.
			 File has to have already been closed */
    void removeFile(int fd){
			if (fd >= 2 && fd < 130){
      	struct thread *t = thread_current();
      	t->fileArray[fd - 2] = NULL;
			}
    }


		/* Opens a file from the filearray with the given file descriptor.
			 returns NULL if the (fd) is out of bounds or there is no file with that (fd) */
    struct file* fdOpen(int fd){
      if (fd >= 2 && fd < 130){
        struct thread *t = thread_current();				
        return t->fileArray[fd - 2];
      } else {
			return NULL;
      }
    }
   














