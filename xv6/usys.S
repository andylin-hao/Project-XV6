#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(hlight)
SYSCALL(swpath)
SYSCALL(retprt)
SYSCALL(cmdmod)
SYSCALL(clrscr)
SYSCALL(getpos)
SYSCALL(setcln)
SYSCALL(lstpos)
SYSCALL(hlline)
SYSCALL(delhlt)
SYSCALL(alarm)
SYSCALL(time)
SYSCALL(outexac)
SYSCALL(kbevent)
SYSCALL(gameget)
SYSCALL(date)
SYSCALL(set)
SYSCALL(get)