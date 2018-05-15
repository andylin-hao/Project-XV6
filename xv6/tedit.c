#include"types.h"
#include"stat.h"
#include"user.h"
#include"fcntl.h"
#include "x86.h"

#define CONTENT_SIZE 80



#define MAX_LINE_NUM 24
#define KEY_UP 0xE2
#define KEY_DOWN 0xE3
#define KEY_LEFT 0xE4
#define KEY_RIGHT 0xE5
#define KEY_ESC 27
#define CRTPORT 0x3d4
#define PAR_SIZE 2048
#define MAX_OUTPUT 80*(MAX_LINE_NUM-1) // last line to print status

#define VIS_MODE 0
#define EDT_MODE 1
#define MV_U 17
#define MV_D 18
#define MV_L 19
#define MV_R 20

char move_u = 17;
char move_d = 18;
char move_l = 19;
char move_r = 20;

char screen[24*80];
int screenpos = 0;

int firstpos = 0;
int lastpos = 0;


struct node
{
  struct node *prev;
  struct node *next;
  char content[CONTENT_SIZE + 1];
  int size;
};


struct node*
  pushback_node(struct node* front)
{
  struct node* new_node;
  new_node = (struct node*)sbrk(sizeof(struct node));
  memset(new_node->content, 0, CONTENT_SIZE+1);
  new_node->size = 0;

  new_node->prev = front;
  new_node->next = front->next;
  front->next = new_node;

  if (new_node->next != 0) {
    new_node->next->prev = new_node;
  }
  return new_node;
}



struct doc
{
  struct node head;
  struct node *last;
  int size;
};


//////////////////////////////////////////////////////////////////////////////////
struct doc f;
//////////////////////////////////////////////////////////////////////////////////

char getca(int pos)
{
  if (pos >= f.size)return 0;
  int npos = (pos - 1) / CONTENT_SIZE;
  int off = (pos - 1) % CONTENT_SIZE;
  struct node* cnode = &f.head;
  for (int i = 0; i < npos; ++i)cnode = cnode->next;
  return cnode->content[off];
}


struct node* setca(int pos, char c)
{
  if (pos>=f.size)
  {
    if (f.last->size<CONTENT_SIZE)f.last->content[f.last->size++] = c;
    else
    {
      if(f.last->next == 0)f.last = pushback_node(f.last);
      else f.last = f.last->next;
      f.last->content[f.last->size++] = c;
    }
    return f.last;
  }
  else
  {
    int npos = (pos-1) / CONTENT_SIZE ;
    int off = (pos - 1) % CONTENT_SIZE;
    struct node* cnode = &f.head;
    for (int i = 0; i < npos; ++i)cnode = cnode->next;
    cnode->content[off] = c;
    return cnode;
  }
}

int addc(int pos, char c)
{
  if (pos >= f.size)
  {
    setca(pos, c);
    f.size++;
  }
  else
  {
    int off = (pos - 1) % CONTENT_SIZE;
    char org = getca(pos);
    struct node* lnode, *cnode;

    lnode = setca(pos, c);
    setca(f.size + 1, getca(f.size));
    f.size++;
    cnode = f.last;

    while (cnode != lnode)
    {
      for (int j = cnode->size - 1; j>0; j--)
        cnode->content[j] = cnode->content[j - 1];
      cnode = cnode->prev;
      if (cnode->next)cnode->next->content[0] = cnode->content[cnode->size - 1];
    }
    for (int j = cnode->size - 1; j>off + 1; j--)
      cnode->content[j] = cnode->content[j - 1];
    if (cnode->next)cnode->next->content[0] = cnode->content[cnode->size - 1];
    if (pos%CONTENT_SIZE)cnode->content[off + 1] = org;
    else cnode->next->content[0] = org;
  }
  return 0;
}

int deletec(int pos)
{
  if (pos == f.size)
  {
    f.last->content[f.last->size - 1] = '\0';
    f.last->size--;
    f.size--;
    if (f.last->size == 0)f.last = f.last->prev;
  }
  else
  {
    

    //int npos = (pos - 1) / CONTENT_SIZE;
    int off = (pos - 1) % CONTENT_SIZE;
    struct node *cnode,*lnode;
    cnode = setca(pos, getca(pos + 1));
    lnode = f.last;
    
    for (int i = off; i < cnode->size-1; i++)
    {
      cnode->content[i] = cnode->content[i+1];
    }
    
    if (cnode == lnode)
    {
      f.last->size--;
      f.size--;
      return 0;
    }
    cnode->content[cnode->size - 1] = cnode->next->content[0];
    cnode = cnode->next;
    while (cnode != lnode)
    {
      for (int i = 0; i < cnode->size - 1; i++)
      {
        cnode->content[i] = cnode->content[i + 1];
      }
      cnode->content[cnode->size - 1] = cnode->next->content[0];
      cnode = cnode->next;
    }
    for (int i = 0; i < cnode->size - 1; i++)
    {
      cnode->content[i] = cnode->content[i + 1];
    }
    cnode->content[cnode->size - 1] = '\0';
    f.last->size--;
    f.size--;
    if (f.last->size == 0)f.last = f.last->prev;

  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int newfile = 0;
/////////////////////////////////////////////////////////////////////////////////////////

int openfile(char *file_name)
{
  int fd;
  fd = open(file_name, O_RDWR);
  if(fd < 0){
    newfile = 1;
    printf(1, "vim: created new file %s.\n", file_name);
    fd = open(file_name, O_CREATE|O_RDWR);
  }
  return fd;
}

int loadfile(int fd)
{
  int n;
  char buf[CONTENT_SIZE+1];
  if(newfile) return 0;
  else
  {
    while(1)
    {
      n = read(fd,buf,CONTENT_SIZE);
      if(n<0)printf(2,"failed to load file\n");
      if(n == 0)break;
      printf(1,"n:%d\n", n);
      if(n<CONTENT_SIZE)n--;
      for (int i = 0; i < n; ++i)
      {
        addc(f.size,buf[i]);
       
      }
    }
    return f.size;
  }
}/**/


void intiscreen()
{
  memset(screen,0,24*80);
  struct node* nnode = &f.head;
  while(1)
  {
  for (int i = 0; i < nnode->size; ++i)
  {
    if( nnode->content[i]!= '\n')
      screen[screenpos++] = nnode->content[i];
    else
    {
      screenpos = screenpos+(80-screenpos%80);
    }
    if(screenpos>=24*80)
    {
      if(screen[0] == 0)firstpos++;
      else
      {
        for (int i = 0; i < 80; ++i)
        {
          if(screen[i])firstpos++;
          else
          {
            firstpos++;
            break;
          }
        }
      }

      memmove(screen,screen+80,23*80);
      memset(screen+23*80,0,80);
      screenpos = 23*80;

    }
  }
  if(nnode == f.last)break;
  nnode = nnode->next;
  }
  return ;
}


int scrpos_to_docpos()
{
  return 0;
}


int
main(int argc, char *argv[])
{

   if(argc<2)
   {
    printf(1,"please input filename\n");
    exit();
   }
   printf(1,"start\n");
   newfile = 0;
   f.head.next = 0;
   f.head.prev = 0;
   f.last = &f.head;
   f.size = 0;
   f.head.size = 0;
   printf(1,"stop1\n");
   memset(f.head.content, 0, CONTENT_SIZE+1);

   printf(1,"stop2\n");
   int fd = openfile(argv[1]);
   printf(1,"stop3\n");
   loadfile(fd);
   close(fd);
   printf(1,"stop4\n");
   struct node* nnode;
   nnode = &f.head;
   
   clrscr();
   //printf(2,"%d\n", f.size);
   while(1)
   {
    printf(1,"%s", nnode->content);
    if(nnode == f.last)break;
    nnode = nnode->next;
   }
   if(cmdmod(CMDTYPE_TEDIT,1)<0)
   {
    printf(2,"set failed\n");
    exit();
   }

   intiscreen();
   //printf(1,"%d",screenpos);

   //int pos;
   while(1)
   {
    int b;
    read(0,&b,1);
    switch(b)
    {
      case KEY_RIGHT:
      case KEY_LEFT:
      //pos = getpos();
      //printf(1,"%d",pos);
      break;
   }
  }
  exit();
}
