#include "types.h"
#include "stat.h"
#include "user.h"

#define         hlight_type_c     1
#define         close_hlight      0

int
main(int argc, char *argv[])
{
  if(argv[1][0] == '-')
  {
		if(argv[1][1] == 'c')
		{
			if(argv[1][2] == '\0')
			{
				if(hlight(hlight_type_c))printf(1,"start hlightting key words in C.\n");
				else printf(1,"failed to change hlight type\n");
			}
			else if(argv[1][2] == 'l')
				if(argv[1][3] == '\0')
				{
					if(hlight(close_hlight))printf(1,"stop hlightting\n");
					else printf(1,"failed to change hlight type\n");
				}
		}
		else if(argv[1][1] == 'h')
		{
			if(argv[1][2] == '\0')
			{
				printf(1,"###############################################################################\n");
				printf(1,"-h show the help \n-c hlight the key words in C\n-cl stop hlightting all\n");
				printf(1,"###############################################################################\n");
			}
		}
		else printf(1,"no valuable parameter found, parameter -h for help\n");
		
  }
  else printf(1,"no valuable parameter found, parameter -h for help\n");
  exit();
}
