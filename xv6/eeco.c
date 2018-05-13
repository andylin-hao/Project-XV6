#include "types.h"
#include "stat.h"
#include "user.h"


int
main(int argc, char *argv[])
{
  	int i;
  	int j;
  	int k;
	int transfer = 0;
	int intrns = 0;
	int pos = 0;
	int changeline = 1;
	if(argv[1][0] == '-')
		if(argv[1][1] == 'e')
			if(argv[1][2] == '\0')
				transfer = 1;
  	for(i = 1+transfer; i < argc; i++)
	{
    		if(transfer)
    		{
    			int sum = 0;
    			int account = 0;
    			for(j = 0;argv[i][j];j++)
					if(argv[i][j] == '"')sum++;

				if(sum<=1) 
				{
					printf(1,"%s", argv[i]);
					printf(1,"%s", i+1 < argc ? " " : "");
					continue;
				}
    			
				for(j = 0;argv[i][j];j++)
				{
					if(argv[i][j] == '"'&& ++account != sum)intrns = intrns^1;
					else if(argv[i][j] == 92 && intrns)
					{
						if(argv[i][j+1] == 'n')argv[i][j+1] = '\n';
						if(argv[i][j+1] == 't')argv[i][j+1] = '\t';
						if(argv[i][j+1] == 'b')
						{
							argv[i][j+1] = 92;
							for (k = j; k >=0; k--)
							{
								if(argv[i][k] !=92)
								{
									argv[i][k] = 92;
									break;
								}
							}
							j++;
						}
						if(argv[i][j+1] == 'c'){argv[i][j+1] = 92;changeline = 0;j++;}
						if(argv[i][j+1] == 92){argv[i][j+1] = 7;j++;}
					}
				}
				for(pos = 0;pos<j;pos++)
    				if(argv[i][pos] != 92&&argv[i][pos] != '"')
    				{
    					if(argv[i][pos] != 7)
    						printf(1,"%c",argv[i][pos]);
    					else
    						printf(1,"\\");
    				}
    			if(argv[i][j-1] == '"'&&sum%2)printf(1,"\"");
    		}
    		else
    		{
    			printf(1,"%s", argv[i]);
    		}
    		printf(1,"%s", i+1 < argc ? " " : "");
	}
	if(changeline)printf(1,"\n");

  	exit();
}
