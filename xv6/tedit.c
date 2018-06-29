#include"types.h"
#include"stat.h"
#include"user.h"
#include"fcntl.h"
#include "x86.h"

#define CONTENT_SIZE 80



#define BACKSPACE 8
#define TAB 9
#define MAX_LINE_NUM 24
#define KEY_UP 0xE2
#define KEY_DOWN 0xE3
#define KEY_LEFT 0xE4
#define KEY_RIGHT 0xE5
#define KEY_ESC 27
#define CRTPORT 0x3d4
#define PAR_SIZE 2048
#define MAX_OUTPUT 80*(MAX_LINE_NUM-1) // last line to print status
#define C(x)  ((x)-'@')

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

int linenum = 0;
int nowline = 0;
int firstline_onscreen = 0;
int ese_mode = 0;
int hlight_mode = 0;


int edit_mode = 0;

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
  new_node = (struct node*)malloc(sizeof(struct node));
  memset(new_node->content, 0, CONTENT_SIZE + 1);
  new_node->size = 0;

  new_node->prev = front;
  new_node->next = front->next;
  front->next = new_node;

  if (new_node->next != 0) {
    new_node->next->prev = new_node;
  }
  return new_node;
}

struct node*
  add_node(struct node* front)
{
  struct node* new_node;
  new_node = (struct node*)malloc(sizeof(struct node));
  memset(new_node->content, 0, CONTENT_SIZE + 1);
  new_node->size = 0;

  new_node->next = 0;
  new_node->prev = 0;

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


int newpara(struct node* now,int pos)
{
  struct node* newnode;
  newnode = add_node(now);
  for (int i = pos; i < now->size; i++)
  {
    newnode->content[i - pos] = now->content[i];
  }
  now->content[pos] = '\n';
  
  memset(now->content + pos + 1, 0, CONTENT_SIZE - pos - 1);
  newnode->size = now->size - pos;
  now->size = pos + 1;

  if (newnode->content[newnode->size - 1] == '\n' || f.last == now)
  {
    if (f.last == now)f.last = now->next;
    return 1;
  }
  while (newnode->content[newnode->size - 1] != '\n' && newnode->content[newnode->size - 1] != '\0')
  {
    if (newnode == f.last)break;

    if(newnode == 0)break;
    
    int len = CONTENT_SIZE - newnode->size;
    if (newnode->next->size < CONTENT_SIZE - newnode->size)
    {
      len = newnode->next->size;
      for (int i = 0; i < len; i++)
      {
        newnode->content[newnode->size++] = newnode->next->content[i];
      }
      struct node* un = newnode->next;
      memset(un->content, 0, CONTENT_SIZE + 1);
      un->size = 0;
      if(un != f.last)
      {
        newnode->next = un->next;
        un->next->prev = newnode;
        
        un->next = 0;
        un->prev = 0;

        un->prev = f.last;
        un->next = f.last->next;
        if(f.last->next)f.last->next->prev = un;
        f.last->next = un;
        break;
      }
      else
      {
        f.last = un->prev;
        break;
      }
    }
    else
    {
      for (int i = 0; i < len; i++)
      {
        newnode->content[newnode->size++] = newnode->next->content[i];
      }
      for (int i = len; i < newnode->next->size - len; i++)
      {
        newnode->next->content[i - len] = newnode->next->content[i];
      }
      newnode->next->size = newnode->next->size - len;
      memset(newnode->next->content + newnode->next->size, 0, CONTENT_SIZE - newnode->next->size);
      newnode = newnode->next;
    }
  }
  return 0;


}


//add char in specified node and if the node is full before addiction,return the char which is removed
int addc_node(struct node* now, int pos,int c)
{
  if(now == 0)return -1;
  if (pos > CONTENT_SIZE)return -1;
  if (now->size < CONTENT_SIZE)
  {
    if (pos >= now->size)
    {
      now->content[now->size++] = c;
      return 0;
    }
    for (int i = now->size; i > pos; i--)
    {
      now->content[i] = now->content[i - 1];
    }
    now->content[pos] = c;
    now->size++;
    return 0;
  }
  else
  {
    int lef = now->content[now->size - 1];
    for (int i = now->size - 1; i >pos; i--)
    {
      now->content[i] = now->content[i - 1];
    }
    now->content[pos] = c;
    return lef;
  }
}

//add char in specified node
int addc(struct node* now, int pos, int c)
{
  //struct node* cur;
  if(now == 0)return -1;
  if (c == '\n')
  {
    newpara(now, pos);
  }
  else
  {
    while (1)
    {
      c = addc_node(now, pos, c);
      if (c == -1)return -1;
      if (!c)break;
      else if(c == '\n')
      {
        struct node* a;
        a = add_node(now);
        a->content[a->size++] = '\n';
        if(f.last == now)f.last = a;
        break;
      }
      else
      {
        
        if (!now->next)
        {
          add_node(now);
          if (now == f.last)
            f.last = now->next;
        }
        now = now->next;
        pos = 0;
      }
    }
  }
  f.size++;
  return 0;
}



//delete a char in a specified node
int delete_node(struct node* now, int pos)
{
  if(now == 0)return -1;
  if (pos > now->size || now->size == 0)return -1;
    for (int i = pos; i < now->size-1; i++)
    {
      now->content[i] = now->content[i + 1];
    }
    now->content[now->size - 1] = '\0';
    now->size--;

    if (now->size)
    {
      if (now->content[now->size - 1] != '\n'&& now->content[now->size - 1] != '\0')
        return 1;
      else return 0;
    }
    else
    {
      if (now != f.last)
      {
        now->prev->next = now->next;
        now->next->prev = now->prev;

        now->next = 0;
        now->prev = 0;

        now->prev = f.last;
        now->next = f.last->next;
        if (f.last->next)f.last->next->prev = now;
        f.last->next = now;
      }
      else
      {
        f.last = now->prev;
      }
      return 0;
    }
}


// merge node
int merge_para(struct node* now,int pos)
{
  if(now == 0)return -1;
  struct node* un;
  if (now == f.last)now->content[--now->size] = '\0';
  while(now != f.last)
  {
    if (CONTENT_SIZE - pos >= now->next->size)
    {
      for (int i = 0; i < now->next->size; i++)
      {
        now->content[i + pos] = now->next->content[i];
        now->size = i + pos + 1;
      }
      un = now->next;
      memset(un->content, 0, CONTENT_SIZE + 1);
      un->size = 0;

      if (un != f.last)
      {
        now->next = un->next;
        un->next->prev = now;

        un->next = 0;
        un->prev = 0;

        un->next = f.last->next;
        un->prev = f.last;
        if (f.last->next)f.last->next->prev = un;
        f.last = un;

      }
      else
      {
        f.last = un->prev;
      }
      break;
    }
    else
    {
      for (int i = pos; i < CONTENT_SIZE; i++)
      {
        now->content[i] = now->next->content[i - pos];
        now->size = i + 1;
      }
      for (int i = CONTENT_SIZE-pos; i < now->next->size; i++)
      {
        now->next->content[i - CONTENT_SIZE + pos] = now->next->content[i];
      }
      now->next->size = now->next->size - (CONTENT_SIZE - pos);
      memset(now->next->content + now->next->size, 0, CONTENT_SIZE - now->next->size);
      now = now->next;
      if (now->content[now->size - 1] == '\n')break;
      else pos = now->size;
    }
  }
  return 0;
}


//delete a char in specified node
int deletec(struct node* now, int pos)
{
  if(now == 0)return -1;
  if (pos >= now->size)return -1;
  if (now->content[pos] != '\n')
  {
    if (!delete_node(now, pos))
    {
      f.size--;
      return 1;
    }
    else
    {
      while (now != f.last)
      {
        now->content[now->size++] = now->next->content[0];
        now = now->next;
        if (delete_node(now, 0))
        {
          now = now->next;
        }
        else
        {
          break;
        }
      }
    }
  }
  else
  {
    merge_para(now,pos);
  }
  f.size--;
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
int newfile = 0;
/////////////////////////////////////////////////////////////////////////////////////////


//open the file whose name is *filename
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

//load the content of the file to a linked list
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
      if(n<0)
      {
        printf(2,"failed to load file\n");
        return -1;
      }
      if(n == 0)break;
      //printf(1,"n:%d\n", n);
      //if(n<CONTENT_SIZE)n--;
      for (int i = 0; i < n; ++i)
      {
        addc(f.last,f.last->size,buf[i]);
       
      }
    }
    return f.size;
  }
}/**/



//clear the screen
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




// int scrpos_to_docpos()
// {
//   return 0;
// }

//get the line of the cursor in linked list
struct node* getpline(int pos)
{
  struct node* lineptr = &f.head;
  int linenumber = firstline_onscreen + pos/80;
  for (int i = 0; i < linenumber; ++i)lineptr = lineptr->next;
  return lineptr;
}


//save
void savefile(char* filename)
{
  unlink(filename);
  int fd = open(filename, O_CREATE|O_RDWR);
  struct node* nline;
  nline = &f.head;
  //int n;
  while(1)
  {
    //char a = 'a';
    for (int i = 0; i < nline->size+1&&nline->content[i]; ++i)
    {
      write(fd,(nline->content+i),1);
    }
    if(nline == f.last)break;
    nline = nline->next;
  }
  close(fd);
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


   //init
   linenum = 0;
   nowline = 0;
   newfile = 0;
   f.head.next = 0;
   f.head.prev = 0;
   f.last = &f.head;
   f.size = 0;
   f.head.size = 0;
   edit_mode = 0;
   hlight_mode = 0;

   printf(1,"stop1\n");
   memset(f.head.content, 0, CONTENT_SIZE+1);

   printf(1,"stop2\n");
   int fd = openfile(argv[1]);
   printf(1,"stop3\n");
   if(loadfile(fd)<0)
    {
      close(fd);
      return 0;
    };
   close(fd);
   printf(1,"stop4\n");
   //struct node* nnode;
   //nnode = &f.head;
   
   clrscr();
   //printf(2,"%d\n", f.size);
   
   struct node* count;
   for(count = &f.head;count != f.last;count = count->next)linenum++;
   if(linenum>24) firstline_onscreen = linenum-24;
   count = getpline(0);
   int cc = 1;
   
	//show the content 
   while(1)
   {
    setcln(cc,count->content);
    if(hlight_mode)hlline(cc);
    if(count == f.last||cc>=24*80)break;
    count = count->next;
    cc+=1;
   }
   if(cmdmod(CMDTYPE_TEDIT,1)<0)
   {
    printf(2,"set failed\n");
    exit();
   }

   //handle input here
   while(1)
   {
    int b;
    read(0,&b,1);
    switch(b)
    {
      //case KEY_UP:
       //pos = getpos();
       //if(pos<80)
      //case KEY_LEFT:
      //pos = getpos();
      //printf(1,"%d",pos);
      case C('P'):
      {
        if(!hlight_mode)
        {  
          hlight_mode = 1;
          for (int i = 1; i < 25; ++i)hlline(i);
        }
        else
        {
          hlight_mode = 0;
          for (int i = 1; i < 25; ++i)delhlt(i);
        }
        break;
      }
      case '\n':
      {
        if(edit_mode)
        {//int pos ;
                int lastpos ;
                lastpos = lstpos();
                if(lastpos>=23*80)
                  {
                    //printf(1,"shit");
                    firstline_onscreen++;
                    lastpos -=80;
                  }
                struct node* nline;
                nline = getpline(lastpos);
                addc(nline,lastpos%80,b);
                while(1)
                {
                  setcln(lastpos/80+1,nline->content);
                  if(hlight_mode)hlline(lastpos/80+1);
                  lastpos+=80;
                  if(lastpos>=80*24)break;
                  if(nline == f.last)break;
                  nline = nline->next;
                }
                break;
        }
      }
      case KEY_RIGHT:
        break;
      case KEY_LEFT:
        break;
      case KEY_UP:
      {
        int pos;
        pos = getpos();
        if(pos<80)
        {
          firstline_onscreen--;
          struct node* nline;
          nline = getpline(pos);
          while(1)
            {
              setcln(pos/80+1,nline->content);
              if(hlight_mode)hlline(pos/80+1);
              pos+=80;
              if(pos>=80*24)break;
              if(nline == f.last)break;
              nline = nline->next;
            }
        }
        break;
      }
      case KEY_DOWN:
      {
        int pos;
        pos = getpos();
        if(pos>=23*80)
        {
          struct node* nline;
          nline = getpline(pos);
          if(nline != f.last)
          {
            firstline_onscreen++;
            nline = getpline(0);
            pos = 0;
            //clrscr();
            while(1)
            {
              setcln(pos/80+1,nline->content);
              if(hlight_mode)hlline(pos/80+1);
              pos+=80;
              if(pos>=80*24)break;
              if(nline == f.last)break;
              nline = nline->next;
            }
          }
        }
        break;
      }
      case BACKSPACE:
      {
        if(!edit_mode)break;
        //printf(1,"BACKSPACE");
        int pos;
        pos = getpos();
        struct node* nline;
        nline = getpline(pos);
        /*if(nline->size<80)
        {
            deletec(nline,pos%80);
            setcln(pos/80+1,nline->content);
            if(hlight_mode)hlline(pos/80+1);
        }
        else
        {*/
            deletec(nline,pos%80);
            while(1)
            {
              setcln(pos/80+1,nline->content);
              if(hlight_mode)hlline(pos/80+1);
              pos+=80;
              if(pos>=80*24)break;
              if(nline == f.last)
                {
                  char zero[80];
                  memset(zero,0,80);
                  if(pos<80*24)
                  {
                    while(pos<80*24)
                    {
                      setcln(pos/80+1,zero);
                      pos +=80;
                    }
                  }
                  break;
                }
              nline = nline->next;
            }
        //}
        break;
      }
      case TAB:
      {
        if(!edit_mode)break;
        
        int pos ;
        pos = getpos()-4;
        
        struct node* nline;
        nline = getpline(pos);
        if(nline == 0)
        {
          add_node(f.last);
          f.last = f.last->next;
          nline = f.last;
        }
        for (int i = 0; i < 4; ++i)
        {
            addc(nline,pos%80,' ');
        }
        if(nline->size<80)
          {
            setcln(pos/80+1,nline->content);
            if(hlight_mode)hlline(pos/80+1);
          }
        else
        {
            while(1)
            {
              setcln(pos/80+1,nline->content);
              if(hlight_mode)hlline(pos/80+1);
              pos+=80;
              if(pos>=80*24)break;
              if(nline == f.last)break;
              nline = nline->next;
            }
        }
        break;
      }
      case KEY_ESC:
      {
        edit_mode = 0;
        ese_mode = 1;
        //savefile(argv[1]);
        //clrscr();
        //cmdmod(CMDTYPE_CONSOLE,1);
        //exit();
      } 
      default:
      {
        if(b == 'i'&&edit_mode == 0)
        {
          edit_mode = 1;
          break;
        }
        if(b == ':'&&edit_mode == 0&&ese_mode == 1)
        {
          goto loop10;
        }
        if(!edit_mode)break;
        int pos ;
        pos = getpos()-1;
        
        struct node* nline;
        nline = getpline(pos);
        if(nline == 0)
        {
          add_node(f.last);
          f.last = f.last->next;
          nline = f.last;
        }
        //printf(1,"%d",pos);
        //printf(1,"2");
        if(nline->size<80)
        {
            addc(nline,pos%80,b);
            setcln(pos/80+1,nline->content);
            if(hlight_mode)hlline(pos/80+1);
            //clrscr();
        }
        else
        {
            addc(nline,pos%80,b);
            //printf(1,"%d\n",pos);
            while(1)
            {
              setcln(pos/80+1,nline->content);
              if(hlight_mode)hlline(pos/80+1);
              pos+=80;
              if(pos>=80*24)break;
              if(nline == f.last)break;
              nline = nline->next;
            }
        }
        break;
      }
   }
  }

  //the ending page
  loop10:clrscr();
  char order[80];
  memset(order,0,80);
  order[0] = ':';
  printf(1,"%s",order);
  int ordlen = 1;
  int wrong_order = 0;
  while(1)
  {
    int b;
    read(0,&b,1);
    switch(b)
    {
      case '\n':
      {
        if(wrong_order)
        {
          clrscr();
          memset(order,0,80);
          order[0] = ':';
          printf(1,"%s",order);
          ordlen = 1;
          wrong_order = 0;
          break;
        }
        if(order[1]=='q'&&order[2] == '\0')
        {
          printf(1,"not save\n");
          clrscr();
          cmdmod(CMDTYPE_CONSOLE,1);
            exit();
        }
        else if(order[1] == 'w'&&order[2] == 'q'&&order[3] == '\0')
        {
          printf(1,"save\n");
          savefile(argv[1]);
          clrscr();
          cmdmod(CMDTYPE_CONSOLE,1);
            exit();
        }
        else{
          clrscr();
          wrong_order = 1;
          printf(1,"no match order,please input again,press enter to reinput");
        }
        break;
      }
      case KEY_RIGHT:
      case KEY_LEFT:
      case KEY_UP:
      case KEY_DOWN:
      case BACKSPACE:
        break;
      default:
      {
          order[ordlen++] = b;
          setcln(1,order);
          break;
      }

    }
  }

  exit();
}
