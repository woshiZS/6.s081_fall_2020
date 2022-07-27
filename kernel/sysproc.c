#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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

  backtrace();
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

uint64
sys_sigalarm(void){
  int interval;
  uint64 func_pointer;

  if(argint(0, &interval) < 0 || argaddr(1, &func_pointer) < 0)
    return -1;
  struct proc* p = myproc();
  p->interval = interval;
  p->inter_func = (void(*)())func_pointer;
  return 0;
}

uint64
sys_sigreturn(void){
  // resume register to trapframe and set state to 0.
  struct proc* p = myproc();
  p->trapframe->ra = p->ura;
  p->trapframe->sp = p->usp;
  p->trapframe->gp = p->ugp;
  p->trapframe->tp = p->utp;
  p->trapframe->t0 = p->ut0;
  p->trapframe->t1 = p->ut1;
  p->trapframe->t2 = p->ut2;
  p->trapframe->s0 = p->us0;
  p->trapframe->s1 = p->us1;
  p->trapframe->a0 = p->ua0;
  p->trapframe->a1 = p->ua1;
  p->trapframe->a2 = p->ua2;
  p->trapframe->a3 = p->ua3;
  p->trapframe->a4 = p->ua4;
  p->trapframe->a5 = p->ua5;
  p->trapframe->a6 = p->ua6;
  p->trapframe->a7 = p->ua7;
  p->trapframe->s2 = p->us2;
  p->trapframe->s3 = p->us3;
  p->trapframe->s4 = p->us4;
  p->trapframe->s5 = p->us5;
  p->trapframe->s6 = p->us6;
  p->trapframe->s7 = p->us7;
  p->trapframe->s8 = p->us8;
  p->trapframe->s9 = p->us9;
  p->trapframe->s10 = p->us10;
  p->trapframe->s11 = p->us11;
  p->trapframe->t3 = p->ut3;
  p->trapframe->t4 = p->ut4;
  p->trapframe->t5 = p->ut5;
  p->trapframe->t6 = p->ut6;
  p->trapframe->epc = p->uepc;
  // set state to 0.
  p->isinhandler = 0;
  return 0;
}