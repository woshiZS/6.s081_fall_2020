#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// trace system call before returning to user mode
uint64
sys_trace(void){
  int mask;

  if(argint(0, &mask) < 0)
    return -1;
  
  // no need to add an extra function in proc.c
  // just remeber this argument in a variable
  struct proc* p = myproc();
  p->mask = mask;
  return 0;
}

// show related information about the system
uint64
sys_sysinfo(void){
  // read argument
  uint64 info_ptr;

  if(argaddr(0, &info_ptr) != 0)
    return -1; 
  struct sysinfo kernel_info;
  kernel_info.freemem = free_mem_num();
  kernel_info.nproc = proc_nums();
  // just read information is okay?
  struct proc *p = myproc();
  if(copyout(p->pagetable, info_ptr, (char *)&kernel_info, sizeof(kernel_info)) < 0)
    return -1;
  return 0;
}