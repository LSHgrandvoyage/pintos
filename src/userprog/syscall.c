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

typedef int pid_t;

static void syscall_handler (struct intr_frame *);

void halt (void);
void exit (int status);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);
bool create (const char *file, unsigned initial_size);
int write (int fd, const void *buffer, unsigned size);

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
syscall_handler (struct intr_frame *f) 
{
  uint32_t *esp = (uint32_t *)f->esp;
  check_validation(esp);

  switch (esp[0]){
    case SYS_HALT: 
	halt(); break;
    case SYS_EXIT: 
	check_validation(&esp[1]);
	exit((int)esp[1]); 
	break;  
    case SYS_EXEC: 
        check_validation(&esp[1]);
        f->eax = exec((const char *)esp[1]);
        break;
    case SYS_WAIT:
        check_validation(&esp[1]);
	f->eax = wait((pid_t)esp[1]);
	break;
    case SYS_CREATE:
	check_validation((const void *)esp[1]);
	check_validation(&esp[2]);
	f->eax = create((const char *)esp[1], (unsigned)esp[2]);
	break;
    case SYS_REMOVE: break;
    case SYS_OPEN: break;
    case SYS_FILESIZE: break;
    case SYS_READ: break;
    case SYS_WRITE:
        check_validation(&esp[1]);
        check_validation(&esp[2]);
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
  if (file == NULL || strlen(file) == 0){
    exit(-1);
  }
  return filesys_create(file, initial_size);
}

int
write (int fd, const void *buffer, unsigned size){
  if (fd == 1){
   putbuf(buffer, size);
   return size;
  }
  return -1;
}
