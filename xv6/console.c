// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.
//0x0700 white
//0x0400 red
//0x300 blue(light)
//0x200	green
//0x100 blue

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
//#include "umalloc.c"

static void consputc(int);

static int panicked = 0;

int lastblank = -1;

int hlighttype = 0;

int isConsole = 1;

int inputPos = 0;

int init = 0; //command struct init parameter

int first = 1; //the first time when up/down is pressed

char* C_Key[] = {"int\0","int*\0","double\0","double*\0",
                 "char\0","char*\0","bool\0","for\0",
                 "while\0","if\0","return\0","static\0",
                 "void\0","do\0","else\0","switch\0",
                 "case\0","break\0","continue\0","#define\0",
                 "default\0"};

char* Command[] = {"cat\0",
                   "echo\0",
                   "forktest\0",
                   "grep\0",
                   "init\0",
                   "kill\0",
                   "ln\0",
                   "ls\0",
                   "mkdir\0",
                   "rm\0",
                   "sh\0",
                   "stressfs\0",
                   "usertests\0",
                   "wc\0",
                   "zombie\0",
                   "hilight\0"};

int Cknum = 21;

#define CMDNUM 16

static struct {
  struct spinlock lock;
  int locking;
} cons;



static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c)
    {
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];
  
  cli();
  cons.locking = 0;
  cprintf("cpu%d: panic: ", cpu->id);
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
#define TAB 9
#define UPARROW 226
#define DOWNARROW 227
#define LEFTARROW 228
#define RIGHTARROW 229
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

int getkey(int pos,int len,char* str2)
{
  int i;
  for (i = 0; i<len; ++i)
  {  
    if(crt[lastblank+1+i]%256 != str2[i]||!str2[i])return 0;
  }
  if(str2[i])return 0;
  return 1;
}

int checkkey(int pos)
{
  if(lastblank == -1)
  {
    lastblank = pos;
    return 0;
  }
  else
  {
    int len;
    len = pos - lastblank-1;

    for (int i = 0; i < Cknum; ++i)
    {
        if(getkey(pos,len,C_Key[i]))return 1;
    }
    return 0;
  }
}

int dkey(int pos)
{
  if(crt[pos-1]/256 == 4)return 1;
  else return 0;
}


//hlight the c key words
static void
cgaputc_h(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
  {
    if(checkkey(pos))
    {
      for (int i = lastblank+1; i < pos; ++i)
      {
        crt[i] = (crt[i]%256)|0x0400;
      }
    }
    pos += 80 - pos%80;
    lastblank = pos-1;
  }
  else if(c == BACKSPACE)
  {
    if(pos > 0) pos = pos-1;
    
    if(dkey(pos))//delete hlightting
    {
      int i;
      for (i = pos-1; crt[i]/256 == 4; --i)
      {
        crt[i] = crt[i]%256|0x0700;
      }
      lastblank = i;
    }
    
    if(crt[pos]%256 == ' ')//change lastblank
    {
      int i;
      for (i = pos-2; crt[i]%256 != 32; --i);
      lastblank = i; 
      //crt[23*80+70] = ((ushort)(pos - lastblank)+'0')|0x0700;
    }
  }
  else if(c == ' ')
  {
    if(!checkkey(pos)) crt[pos++] = (c&0xff)|0x0700;
    else
    {
      for (int i = lastblank+1; i < pos; ++i)
      {
        crt[i] = (crt[i]%256)|0x0400;
      }
      crt[pos++] = (c&0xff)|0x0700;
    }
    lastblank = pos-1;
  }
  else {
      crt[pos++] = (c&0xff)|0x0700;
  }
  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }
  
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

static void
matchCommand(char* src, char* match[], int* num)
{
    int count = 0;
    for(int i = 0; i < CMDNUM; i++)
    {
        int j = 0;
        for(; src[j] && Command[i][j]; j++)
        {
            if(src[j] == Command[i][j])
                continue;
            else
                break;
        }
        if(!src[j])
        {
            match[count] = Command[i];
            count++;
        }
    }
    *num = count;
}


//normal put out
static void
cgaputc(int c)
{
  int pos;
  int move = 0;
  
  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(inputPos < 0) {
        for (int i = pos - 1; i % 80 ; ++i) {
            *(crt + i) = *(crt + i + 1);
        }
    }
    if(pos > 0) --pos;
  }
  else if(c == LEFTARROW){
      move = -1;
  }
  else if(c == RIGHTARROW){
      if(isConsole) {
        if(crt[pos] != (' ' | 0x0700) || crt[pos + 1] != (' ' | 0x0700))
            move = 1;
      }
      else {
        if(crt[pos+1] != (0|0x0700))
            move = 1;
      }
  }
  else {
      if(inputPos < 0){
          for (int i = 80 * (pos / 80 + 1); i >= pos + 1 ; --i) {
              *(crt + i) = *(crt + i - 1);
          }
      }
      crt[pos++] = (c&0xff) | 0x0700;  // black on white
  }
  
  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  if(isConsole) {
    int j;
    int start = pos - pos % 80;
    for (j = start; j < pos; ++j) {
        if(crt[j] == (':' | 0x0700))
            break;
    }
    if((pos + move - (j - start)) % 80) {
        pos += move;
        inputPos += move;
    }
  }
  else {
      if(pos+move>=0) {
          pos+=move;
          inputPos +=move;
      }
  }
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  if(!inputPos)
      crt[pos] = ' ' | 0x0700;
}


void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }
  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  if(hlighttype)
      cgaputc_h(c);
  else
      cgaputc(c);
}

#define INPUT_BUF 128
struct {
  struct spinlock lock;
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;

#define COMMAND_BUF 128
struct commandLine{
    char buf[INPUT_BUF];
    struct commandLine* next;
    struct commandLine* pre;
};

struct {
    struct commandLine lines[COMMAND_BUF];
    struct commandLine* head;
    struct commandLine* tail;
    struct commandLine* current;
    uint size;
} command;

#define C(x)  ((x)-'@')  // Control-x

void
consoleintr(int (*getc)(void))
{
  int c;

  acquire(&input.lock);
  if(!init){
      command.head = command.lines;
      command.tail = command.head;
      command.current = command.head;
      command.size = 0;
      init = 1;
  }
  while((c = getc()) >= 0)
  {
    switch(c)
    {
      case C('P'):  // Process listing.
        procdump();
        break;
      case C('U'):  // Kill line.
        while(input.e != input.w &&
              input.buf[(input.e-1) % INPUT_BUF] != '\n')
        {
          input.e--;
          consputc(BACKSPACE);
        }
        break;
      case C('H'): case '\x7f':  // Backspace
        if(isConsole) {
            if(input.e + inputPos != input.w) {
              if(inputPos < 0){
                  for (int i = (input.e % INPUT_BUF) + inputPos - 1; i < (input.e - 1) % INPUT_BUF; ++i) {
                      input.buf[i] = input.buf[i + 1];
                  }
              }
              input.e--;
              consputc(BACKSPACE);
              //cprintf("bakc");
            }
          }
        else
            consputc(BACKSPACE);
        break;
      case TAB: //Tab
          if(input.e-input.r < INPUT_BUF && isConsole)
          {
              int endPos = input.e + inputPos;
              int pos = endPos - input.r;
              endPos--;
              char foreInput[INPUT_BUF];
              foreInput[pos] = '\0';
              while(pos) { //read old input
                  pos = endPos - input.r;
                  foreInput[pos] = input.buf[endPos];
                  endPos--;
              }

              int matchNum = 0;
              char* matchResult[CMDNUM] = {0};
              matchCommand(foreInput, matchResult, &matchNum); //match compatitable command

              if(matchNum == 1) { //case for one match
                  int length = strlen(matchResult[0]) - (input.e + inputPos - input.r) + 1;
                  for (int i = input.e % INPUT_BUF - 1 + length; i >= input.e % INPUT_BUF + inputPos + length ; --i) {
                      input.buf[i] = input.buf[i - length];
                  }
                  for(int i = input.e + inputPos - input.r; matchResult[0][i]; i++) {
                      input.buf[(input.e + inputPos) % INPUT_BUF] = matchResult[0][i];
                      input.e++;
                      consputc(matchResult[0][i]);
                  }
                  input.buf[(input.e + inputPos) % INPUT_BUF] =' ';
                  input.e++;
                  consputc(' ');
              }
              else{ // case for multiple commands
                  consputc('\n');
                  for(int i = 0; i < matchNum; i++) {
                      for(int j = 0; matchResult[i][j]; j++)
                          consputc(matchResult[i][j]);
                      consputc(' ');
                  }
                  while(input.e != input.w &&
                        input.buf[(input.e-1) % INPUT_BUF] != '\n') {
                      input.e--;
                  }
                  input.buf[input.e++ % INPUT_BUF] = '\n';
                  consputc('\n');
                  input.w = input.e;
                  inputPos = 0;
                  wakeup(&input.r);
              }
          }
          break;
      case LEFTARROW: //left arrow
          if(isConsole)
              cgaputc(LEFTARROW);
          else {
            cgaputc(LEFTARROW);
            input.buf[input.e++ % INPUT_BUF] = c;
            input.w = input.e;
            wakeup(&input.r);
          }
          break;
      case RIGHTARROW:
          if(isConsole)
              cgaputc(RIGHTARROW);
          else {
            cgaputc(RIGHTARROW);
            input.buf[input.e++ % INPUT_BUF] = c;
            input.w = input.e;
            wakeup(&input.r);
          }
          break;
      case UPARROW:
          if(isConsole){
              if(command.size){
                  if(!first)
                      command.current = command.current->pre;
                  else
                      first = 0;

                  while(input.e != input.w &&
                        input.buf[(input.e-1) % INPUT_BUF] != '\n') { //kill line
                      input.e--;
                      consputc(BACKSPACE);
                  }

                  for (int i = 0; command.current->buf[i]; ++i) {
                      input.buf[input.e++ % INPUT_BUF] = command.current->buf[i];
                      consputc(command.current->buf[i]);
                  }

              }
          }
          break;
      case DOWNARROW:
          if(isConsole){
              if(command.size){
                  if(!first)
                      command.current = command.current->next;
                  else
                      first = 0;

                  while(input.e != input.w &&
                        input.buf[(input.e-1) % INPUT_BUF] != '\n') { //kill line
                      input.e--;
                      consputc(BACKSPACE);
                  }

                  for (int i = 0; command.current->buf[i]; ++i) {
                      input.buf[input.e++ % INPUT_BUF] = command.current->buf[i];
                      consputc(command.current->buf[i]);
                  }

              }
          }
          break;
      default:
          if(isConsole) {
            if(c != 0 && input.e-input.r < INPUT_BUF){
                  c = (c == '\r') ? '\n' : c;
                  if(inputPos <  0 && c != '\n'){
                      for (int i = input.e % INPUT_BUF; i > input.e % INPUT_BUF + inputPos ; --i) {
                          input.buf[i] = input.buf[i - 1];
                      }
                      input.buf[input.e % INPUT_BUF + inputPos] = c;
                      input.e++;
                  }
                  else
                      input.buf[input.e++ % INPUT_BUF] = c;
                  consputc(c);
                  if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
                    inputPos = 0;
                    first = 1;
                    input.w = input.e;

                    if(command.size == 0){
                        for (int i = input.r; i < input.e - 1; ++i)
                            command.lines[0].buf[i - input.r] = input.buf[i];
                        command.size++;
                        command.head->pre = command.tail;
                        command.head->next = command.tail;
                    }
                    else if(command.current == command.tail){
                        if(&command.lines[command.size % COMMAND_BUF] == command.head)
                            command.head = command.head->next;
                        for (int i = input.r; i < input.e - 1; ++i)
                            command.lines[command.size % COMMAND_BUF].buf[i - input.r] = input.buf[i];
                        command.tail->next = &command.lines[command.size % COMMAND_BUF];
                        command.tail->next->pre = command.tail;
                        command.tail = command.tail->next;
                        command.tail->next = command.head;
                        command.head->pre = command.tail;
                        command.current = command.tail;
                        command.size++;
                    }
                    else{
                        if(command.current == command.head)
                            command.head = command.head->next;
                        command.tail->next = command.current;
                        command.current->pre = command.tail;
                        command.current->next = command.head;
                        command.head->pre = command.current;
                        command.tail = command.current;
                    }
                    wakeup(&input.r);
                  }
                }
          }
          else {
            if(c != 0 && input.e-input.r < INPUT_BUF) {
                  c = (c == '\r') ? '\n' : c;
                  if(inputPos <  0 && c != '\n'){
                      for (int i = input.e % INPUT_BUF; i > input.e % INPUT_BUF + inputPos ; --i) {
                          input.buf[i] = input.buf[i - 1];
                      }
                      input.buf[input.e % INPUT_BUF + inputPos] = c;
                      input.e++;
                  }
                  else
                      input.buf[input.e++ % INPUT_BUF] = c;
                  consputc(c);
                  if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
                  }
                  input.w = input.e;
                  wakeup(&input.r);
            }
          }
        break;
    }
  }
  release(&input.lock);
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&input.lock);
  while(n > 0){
    while(input.r == input.w){
      if(proc->killed){
        release(&input.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &input.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&input.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");
  initlock(&input.lock, "input");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
}

int getpos()
{
  int pos;
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);
  return pos;
}

void
clearscreen(void)
{
  int pos = 0;
  inputPos = 0;
  for(int i = 0;i<24*80;i++)
	crt[i] = 0|0x700;

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;

}

