#include "types.h"
#include "stat.h"
#include "user.h"

#define MN_SIZE 1024

char buf[512];

int getcontnt(int fd,char* doc,int* size)
{
  int n;
  int off = 0;;
  while((n = read(fd, buf, sizeof(buf))) > 0)
  {  
    //write(1, buf, n);
    //if(off+n>size)
    for(int j = 0;j<n;j++)doc[off+j] = buf[j];
    off += n;
  }
  if(n < 0)
  {
    printf(1, "docedt: read error\n");
    exit();
  }
  return off;
}


int
main(int argc, char *argv[])
{
  int fd;
  int docsize = MN_SIZE;
  char* doc = (char*)malloc(MN_SIZE*sizeof(char));
  if((fd = open(argv[1], 0)) < 0){
      printf(1, "cat: cannot open %s\n", argv[1]);
      exit();
    }
  getcontnt(fd,doc,&docsize);
  printf(1,"%s",doc);
  //setptr(doc,docsize);
  free(doc);
    close(fd);
  exit();
}
