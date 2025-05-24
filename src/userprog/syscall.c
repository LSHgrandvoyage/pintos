#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/directory.h"
#include "threads/palloc.h"
#include "filesys/file.h"

typedef int pid_t;

static void syscall_handler (struct intr_frame *);
bool validate_filename(const char *file);

void halt (void);
void exit (int status);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);

struct file_descriptor {
  int fd;
  struct file *file;
  struct list_elem elem;
};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
check_validation(const void *vaddr){
  if (vaddr == NULL || !is_user_vaddr(vaddr) || !pagedir_get_page(thread_current()->pagedir, vaddr)){
    exit(-1);
  }
}

static void
check_str_validation (const char *str){
  while (true){
    check_validation(str);
    if (*str == '\0') {
      break;
    }
    str++;
  }
}

static void
syscall_handler (struct intr_frame *f) 
{
  uint32_t *esp = (uint32_t *)f->esp;
  check_validation(esp);

  switch (esp[0]){
    case SYS_HALT: 
	halt();
	break;
    case SYS_EXIT:
	check_validation(&esp[1]);
	exit((int)esp[1]); 
	break;  
    case SYS_EXEC: 
	check_validation((const void *)esp[1]);
        f->eax = exec((const char *)esp[1]);
        break;
    case SYS_WAIT:
	f->eax = wait((pid_t)esp[1]);
	break;
    case SYS_CREATE:
	check_validation((const void *)esp[1]);
	f->eax = create((const char *)esp[1], (unsigned)esp[2]);
	break;
    case SYS_REMOVE:
	check_validation((const void *)esp[1]);
	f->eax = remove((const char *)esp[1]);
    case SYS_OPEN: 
	check_validation((const void *)esp[1]);
	f->eax = open((const char *)esp[1]);
	break;
    case SYS_FILESIZE:
	check_validation(&esp[1]);
	f->eax = filesize((int)esp[1]);
	break;
    case SYS_READ: 
	check_validation(&esp[1]);
	check_validation((const void *)esp[2]);
	check_validation(&esp[3]);
	f->eax = read((int)esp[1], (void *)esp[2], (unsigned)esp[3]);
	break;
    case SYS_WRITE:
        check_validation(&esp[1]);
        check_validation((const void *)esp[2]);
        check_validation(&esp[3]);
        f->eax = write((int)esp[1], (void *)esp[2], (unsigned)esp[3]);
        break;
    case SYS_SEEK: break;
    case SYS_TELL: break;
    case SYS_CLOSE: break;
    default: exit(-1);
  }

}

void
halt (void){
  shutdown_power_off();
}

void
exit (int status){
  struct thread *curr = thread_current();
  curr->exit_status = status;
  printf("%s: exit(%d)\n", curr->name, status);
  thread_exit();
}

pid_t
exec (const char *cmd_line){
  check_validation(cmd_line);
  return process_execute(cmd_line);
}

int
wait (pid_t pid){
  return process_wait(pid);
}

bool
create (const char *file, unsigned initial_size){
  if (file == NULL || !validate_filename(file)){
    return false;
  }

  if (filesys_open(file) != NULL){
    return false;
  }

  return filesys_create(file, initial_size);
}

bool
remove (const char *file){
  if (validate_filename(file)){
    return false;
  }
  return filesys_remove(file);
}

int
open (const char *file){
  if (file == NULL || !validate_filename(file)){
    return -1;
  }
  
  struct file *f = filesys_open(file);
  if (f == NULL){
    return -1;
  }

  struct thread *curr = thread_current();
  struct file_descriptor *fd_struct = palloc_get_page(PAL_ZERO);
  if (fd_struct == NULL){
    file_close(f);
    return -1;
  }
  
  fd_struct->fd = curr->next_fd++;
  fd_struct->file = f;
  list_push_back(&curr->fd_table, &fd_struct->elem);

  return fd_struct->fd;
}

int
filesize (int fd){
  struct thread *curr = thread_current();
  struct list_elem *e;

  for (e = list_begin(&curr->fd_table); e != list_end(&curr->fd_table); e = list_next(e)){
    struct file_descriptor *fd_struct = list_entry(e, struct file_descriptor, elem);
    if (fd_struct->fd == fd){
      return file_length(fd_struct->file);
    }
  }
  return -1;
}

int
read (int fd, void *buffer, unsigned size){
  check_validation(buffer);
  struct thread *curr = thread_current();
  struct list_elem *e;
  int i;
  
  if (fd == 0){
    for (i = 0; i < size; i++){
      ((char *)buffer)[i] = input_getc();
    }
  return size;
  }
  
  for (e = list_begin(&curr->fd_table); e != list_end(&curr->fd_table); e = list_next(e)){
    struct file_descriptor *fd_struct = list_entry(e, struct file_descriptor, elem);
    if (fd_struct->fd == fd){
     return file_read(fd_struct->file, buffer, size);
    }
  }
  return -1;
}

int
write (int fd, const void *buffer, unsigned size){
  check_validation(buffer);
  struct thread *curr = thread_current();
  
  if (fd == 1){
   putbuf(buffer, size);
   return size;
  }
  
  struct list_elem *e;
  for (e = list_begin(&curr->fd_table); e != list_end(&curr->fd_table); e = list_next(e)){
    struct file_descriptor *fd_struct = list_entry(e, struct file_descriptor, elem);
    if (fd_struct->fd == fd){
      return file_write(fd_struct->file, buffer, size);
    }
  }
  return -1;
}

bool
validate_filename(const char* file){
  check_str_validation(file);
  int len = strlen(file);
  return len > 0 && len <= NAME_MAX;
}
