#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
check_validation(const void *vaddr){
  if (!is_user_vaddr(vaddr) || !pagedir_get_page(thread_current()->pagedir, vaddr)){
    exit(-1);
  }
}

static void
syscall_handler (struct intr_frame *f) 
{
  uint32_t *esp = (uint32_t *)f->esp;
  
  check_validation(esp);

  switch (esp[0]){
    case SYS_HALT: halt(); break;
    case SYS_EXIT: check_validation(&esp[1]); exit((int)esp[1]); break;  
    case SYS_EXEC: break;
    case SYS_WAIT: break;
    case SYS_CREATE: break;
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

int write (int fd, const void *buffer, unsigned size){
  if (fd == 1){
   putbuf(buffer, size);
   return size;
  }
  return -1;
}
