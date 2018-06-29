#include "types.h"
#include "stat.h"
#include "user.h"

#define NAME_LIMIT        30
#define CONTENT_LIMIT     1000

int legalSym(char t)
{
  if((t>='0'&&t<='9')||(t>='a'&&t<='z')||(t>='A'&&t<='Z')||t=='_')
    return 1;
  return 0;
}

int findVar(char *source, int *start, int *end, int cur)
{
  int startPos=cur, endPos=0;
  int len=strlen(source);
  for(; startPos<len; startPos++)
  {
    if(source[startPos]=='$')
      break;
  }
  if(startPos<len)
  {
    startPos++;
    for(endPos=startPos; endPos<len; endPos++)
    {
      if(!legalSym(source[endPos]))
        break;
    }

  }
  else
  {
    return 0;
  }
  *start=startPos;
  *end=endPos;
  return 1;
}

int
main(int argc, char *argv[])
{
  int i;
  int start=0, end;
  int current=0;
  char varName[NAME_LIMIT];
  char value[CONTENT_LIMIT];
  for(i = 1; i < argc; i++)
  {
    while(current<strlen(argv[i]))
    {
      current=start;
      if(findVar(argv[i], &start, &end, current))
      {

        if(current!=start)
        {
          for(;current<start-1; current++)
          {
            printf(1, "%c", argv[i][current]);
          }
        }
        if(end-start>=NAME_LIMIT-1)
        {
          ;
        }
        else
        {
          int j=0;
          for(j=0; start+j<end; j++)
          {
            varName[j]=argv[i][start+j];
          }
          varName[j]=0;
          if(!get(value, varName))
          {

            for(int k=0; k<strlen(value); k++)
            {
              printf(1,"%c", value[k]);
            }
          }
          start=end;
        }
      }
      else
      {
        for(; current<strlen(argv[i]); current++)
        {
          printf(1, "%c", argv[i][current]);
        }
      }
    }
    printf(1, "%s", i+1 < argc ? " " : "\n");
  }
  //printf(1, "%s%s", argv[i], i+1 < argc ? " " : "\n");
  exit();
}