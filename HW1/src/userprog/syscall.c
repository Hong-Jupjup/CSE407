#include "userprog/syscall.h"
#include "threads/vaddr.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	//printf ("system call!\n");

	int sys_num = *(uint32_t *)(f->esp);
	//printf("%d\n", sys_num);

	if(sys_num < 0) {
		printf("not vaild system call number\n");
		exit(-1);
	}

	switch(sys_num) {
		case SYS_HALT:
			halt();
			break;

		case SYS_EXIT:
			if(!is_user_vaddr(f->esp + 4))
				exit(-1);
			exit(*(uint32_t *)(f->esp + 4));
			break;

		case SYS_EXEC:
			if(!is_user_vaddr(f->esp + 4))
				exit(-1);
			f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_WAIT:
			if(!is_user_vaddr(f->esp + 4))
				exit(-1);
			f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
			break;

		case SYS_READ:
			if(!is_user_vaddr(f->esp + 20) || !is_user_vaddr(f->esp + 24) || !is_user_vaddr(f->esp + 28))
				exit(-1);
			f->eax = read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
			break;

		case SYS_WRITE:
			if(!is_user_vaddr(f->esp + 24) || !is_user_vaddr(f->esp + 28))
				exit(-1);
//			hex_dump(f->esp, f->esp, 100, 1);
			f->eax = write((int)*(uint32_t *)(f->esp + 20), (char *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
			break;

		case SYS_FIBO:
			//printf("\nfibo\n");
			//printf("esp: %p\n", f->esp);
			//hex_dump(f->esp, f->esp, 1000, 1);
			if(!is_user_vaddr(f->esp + 28))
				exit(-1);
			f->eax = fibonacci((int)*(uint32_t *)(f->esp + 28));
			break;

		case SYS_MAX:
			//printf("\nmax\n");
			//printf("esp: %p\n", f->esp);
			//hex_dump(f->esp, f->esp, 1000, 1);
			if(!is_user_vaddr(f->esp + 32) || !is_user_vaddr(f->esp + 36) || !is_user_vaddr(f->esp + 40) || !is_user_vaddr(f->esp + 44))
				exit(-1);
			f->eax = max_of_four_int((int)*(uint32_t *)(f->esp + 32), (int)*(uint32_t *)(f->esp + 36), (int)*(uint32_t *)(f->esp + 40), (int)*(uint32_t *)(f->esp + 44));
			break;
	}
}

void 
halt()
{
	shutdown_power_off();
}

void 
exit(int status)
{
	printf("%s: exit(%d)\n", thread_name(), status);
	thread_current()->exit_status = status;
	thread_exit();
}

pid_t 
exec(const char *file)
{
	return process_execute(file);
}

int 
wait(pid_t pid)
{
	return process_wait(pid);
}

int 
read(int fd, void *buffer, unsigned int length)
{
	int i = 0;
	if(fd == 0) {
		for(i=0; (unsigned int)i<length; i++) {
			if((((char *)buffer)[i] = input_getc()) == '\0') {
				break;
			}
		}
	}

	return i;
}

int 
write(int fd, const void *buffer, unsigned length)
{
	if(fd == 1) {
		putbuf(buffer, length);
		return length;
	}
	return -1;
}

int
fibonacci(int n)
{
	//printf("\nn = %d\n", n);
	int i;
	int a = 1;
	int b = 1;
	int sum = 0;

	if(n == 0)
		return 0;
	if(n == 1)
		return 1;
	if (n == 2)
		return 1;
	for(i=2; i<n; i++) {
		sum = a + b;
		a = b;
		b = sum;
		//printf("\na: %d, b = %d, sum -> %d\n", a,b,sum);
	}
	return sum;
}

int
max_of_four_int(int a, int b, int c, int d)
{
	int arr[4];
	int max = a;

	arr[0] = a;
	arr[1] = b;
	arr[2] = c;
	arr[3] = d;

	for(int i=1; i<4; i++) {
		if(max < arr[i]) {
			max = arr[i];
		}
	}

	return max;
}
