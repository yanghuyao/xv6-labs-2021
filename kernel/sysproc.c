#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
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


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 base,user_addr;
  int len;
  if(argaddr(0,&base)<0||argint(1,&len)<0||argaddr(2,&user_addr)<0)   // 获取传入的参数
  {
    printf("sys_pgaccess: error argument\n");
    return -1;
  }
  if(len>64)                        // 设置页面上限
  {
    printf("sys_pgaccess: len %d can not more than 64\n",len);
    return -1;
  }
  uint64 bitmask=0;                 // 定义返回的mask
  int count=0;                      // 记录这是访问的第几个页面
  struct proc *p=myproc();          // 获取proc
  for(uint64 cur_page=base;(cur_page<base+len*PGSIZE);cur_page+=PGSIZE,count++) // 遍历页面               
  {
    pte_t *pte=walk(p->pagetable,cur_page,0);                         // 获取pte
    if(*pte&PTE_A)                  // 检测是否被访问过
    {
      bitmask|=(1L << count);
      (*pte)&=(~PTE_A);       // 还原PTE_A位
    }else
    {
      // do nothing
    }
  }
  copyout(p->pagetable,user_addr,(char *)&bitmask,sizeof(bitmask));  // 要注意(char *)&bitmask中间有&，否则报错
  return 0;
}
#endif

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
