#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "date.h"
#include "envar.h"

struct enVars enVarContainer;
int initiated=0;

int find(char *name, int length)
{
  int i=0;
  for(i=0; i<enVarContainer.varNum; i++)
  {
    if(enVarContainer.contents[i].nameLength==length)
    {
      int j=0;
      for(j=0; j<length; j++)
      {
        if(enVarContainer.contents[i].name[j]!=name[j])
          break;
      }
      if(j==length)
        break;
    }
  }
  if(i<enVarContainer.varNum)
    return i;
  return -1;
}

int changeEnVar(char *con, int pos)
{
  int fillPos=0;
  for(int i=0; con[i]!=0; i++)
  {
    if(con[i]=='$'&& con[i+1]=='{')
    {
      int j=0;
      for(j=i+1; con[j]!='}'; j++)
        ;
      int foundVar=find(&con[i+2],j-i-2);
      cprintf("%d\n", foundVar);
      i=j;
      if(foundVar==-1)
        return -1;
      else
      {
        for(j=0; j<enVarContainer.contents[foundVar].size; j++)
        {
          if(fillPos<CONTENT_LIMIT-1)
          {
            enVarContainer.contents[pos].value[fillPos]=enVarContainer.contents[foundVar].value[j];
            fillPos++;
          }
          else
          {
            return -1;
          }
        }

      }
    }
    else
    {
      if(fillPos<CONTENT_LIMIT-1)
      {
        enVarContainer.contents[pos].value[fillPos]=con[i];
        fillPos++;
      }
      else
      {
        return -1;
      }
    }
  }
  enVarContainer.contents[pos].value[fillPos]=0;
  enVarContainer.contents[pos].size=fillPos;
  return 0;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_alarm(void)
{
  int ticks;
  void (*handler)();

  if(argint(0, &ticks) < 0)
    return -1;
  if(argptr(1, (char**)&handler, 1) < 0)
    return -1;
  proc->alarmticks = ticks;
  proc->alarmhandler = handler;
  return 0;
}
int sys_time(void)
{
  int time = proc->curalarmtick;
  return time;
}

int sys_kbevent(void)
{
  void (*handler)();
  if(argptr(0, (char**)&handler, 1) < 0)
      return -1;
  proc->keyboardHandler = handler;
  return 0;
}

int sys_ingame(void)
{
    int type;
    if(argint(0, &type) < 0)
        return -1;
    isConsole = type;
    return 0;
}

int sys_date(struct rtcdate * val)
{
  if(argptr(0,(void*)&val, sizeof(*val))<0)
  {
    return -1;
  }
  cmostime(val);
  return 0;
}

int sys_set(char *source)
{
  char *con;
  int length=0;
  if(!initiated)
  {
    initiated=1;
    enVarContainer.varNum=0;
  }

  if((length=argstr(0, &con))<0)
  {
    return -1;
  }
  //cprintf("%s\n", con);
  int equalPos=0;
  for(; equalPos<length; equalPos++)
  {
    if(con[equalPos]=='=')
      break;
  }
  if(equalPos>=NAME_LIMIT-1)
    return -1;
  int i=find(con, equalPos);

  if(i==-1)
  {
    i=enVarContainer.varNum;
    if(enVarContainer.varNum==VAR_LIMIT)
      return -1;
    else
    {
      int j=0;
      for(j=0; j<equalPos; j++)
      {
        enVarContainer.contents[i].name[j]=con[j];

      }
      enVarContainer.contents[i].name[j]=0;
      enVarContainer.contents[i].nameLength=j;
      changeEnVar(&con[equalPos+1], i);
      enVarContainer.varNum++;
    }
  }
  else
  {
    changeEnVar(&con[equalPos+1], i);
  }
  //cprintf("%d\n", enVarContainer.varNum);
  return 0;
}

int sys_get(char *content, char *name)
{
  if(argptr(0,(void*)&content, sizeof(*content))<0)
  {
    return -1;
  }
  int length=0;
  if((length=argstr(1, &name))<0)
  {
    return -1;
  }
  int i=find(name, length);
  if(i!=-1)
  {
    int j=0;
    for(j=0; j<enVarContainer.contents[i].size; j++)
    {
      content[j]=enVarContainer.contents[i].value[j];
    }
    content[j]=0;
  }
  else
  {
    return -1;
  }
  return 0;
}
