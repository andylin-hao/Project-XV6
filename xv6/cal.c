#include "types.h"
#include "stat.h"
#include "user.h"

typedef unsigned char uchar;

#define DIV_BY_ZERO -1
#define WRONG_FORM -2 // not a correct seq
#define SYM_FLAG 0
#define NUM_FLAG 1
#define BAD_INPUT -3
#define GOOD_INPUT 3
#define GOOD_RESULT 4
#define ERROR 5
#define SUCCESSFUL_OP 6
#define FAIL_OP 7

struct stack
{
	int tag;
	int size;
	int limit;
	union
	{
		char sym[500];
		float num[500];
	};
	union
	{
		char *cTop;
		float *fTop;
	};
};

struct stack* initStack(int tag)
{
	
	struct stack* container=(struct stack*)malloc(sizeof(struct stack));
	container->tag=tag;
	container->size=0;
	container->limit=500;
	switch(tag)
	{
	case NUM_FLAG:
		container->fTop=container->num;
		break;
	case SYM_FLAG:
		container->cTop=container->sym;
		break;
	default:
	 	break;
	}
}

int pushIn(struct stack *container, void *val)
{
	
	switch(container->tag)
	{
	case NUM_FLAG:
		*container->fTop=*(float*)val;
		container->fTop++;
		container->size++;
		break;
	case SYM_FLAG:
		*container->cTop=*(char*)val;
		container->cTop++;
		container->size++;
		break;
	default:
		return FAIL_OP;
	}
	
	return SUCCESSFUL_OP;
}

void *popOut(struct stack* container)
{
	if(container->size>0)
	{
		switch(container->tag)
		{
		case NUM_FLAG:
			container->fTop--;
			container->size--;
			return container->fTop;
		case SYM_FLAG:
			container->cTop--;
			container->size--;
			return container->cTop;
		default:
			return 0;
		}	
	}
	return 0;
}

int singleCal(float a, float b, char op, float *res)
{
	switch(op)
	{
	case '+':
		*res=a+b;
		return GOOD_RESULT;
	case '-':
		*res=a-b;
		return GOOD_RESULT;
	case '*':
		*res=a*b;
		return GOOD_RESULT;
	case '/':
		if(b==0)
		{
			return DIV_BY_ZERO;
		}
		else
		{
			*res=a/b;
			return GOOD_RESULT;
		}
	default:
		return ERROR;
	}
	return ERROR;
}

int strToFlo(char *source, float *result, int begin)
{
	int length=0;
	int pointPos=-1;
	int neg=0;
	for(length=begin; source[length]!=0; length++)
	{
		if(source[length]=='.')
		{
			if(pointPos==-1)
			{
				pointPos=length;
			}
			else
			{
				return BAD_INPUT;
			}
		}
		else if(source[length]=='-')
		{
			if(!neg&&length==begin)
			{
				neg=1;
			}
			else
			{
				break;
			}
		}
		else if(source[length]=='+')
		{
			if(length!=begin)
			{
				break;
			}
		}
		else if(source[length]<'0'||source[length]>'9')
		{
			break;
		}
	}

	int assist=0;
	for(int i=begin; i<length; i++)
	{
		if(source[i]>='0'&&source[i]<='9')
		{
			assist=assist*10+source[i]-'0';
		}
	}
	//printf(1,"%d\n", assist);
	float res=assist;
	
	if(pointPos!=-1)
	{
		for(int i=0; i<length-pointPos-1; i++)
		{
			res/=10;
		}
	}
	if(neg)
	{
		res=-res;
	}
	*result=res;
	//printf(1,"%f\n", res);
	return length;

}

int calculate(int argc, char *source, float *result)
{
	struct stack* numContainer=initStack(NUM_FLAG);
	struct stack* symContainer=initStack(SYM_FLAG);
	float *input, *singleRes;
	int state, nextPos, flag;
	float *a=0, *b=0;
	char *op=0;
	for(int i=0; source[i]!=0; i++)
	{
		
		//printf(1,"%c\n", source[i]);
		if(source[i]>='0'&&source[i]<='9')
		{
			if((nextPos=strToFlo(source, input, i))!=BAD_INPUT)
			{
				pushIn(numContainer, input);
				i=nextPos-1;
			}
			else
			{
				return BAD_INPUT;
			}
		}
		else
		{
			switch(source[i])
			{
			case '{':
				pushIn(symContainer, &source[i]);
				break;
			case '+':
			case '-':
				
				if(i==0||((source[i-1]>'9'||source[i-1]<'0')&&source[i-1]!='}'))
				{
					if((nextPos=strToFlo(source, input, i))!=BAD_INPUT)
					{
						
						pushIn(numContainer, input);
						i=nextPos-1;
					}
					else
					{
						
						return BAD_INPUT;
					}
				}
				else
				{
					if(symContainer->size==0)
					{
						
						pushIn(symContainer, &source[i]);
					}
					else
					{
						
						op=(char*)popOut(symContainer);
						if(*op!='{')
						{
							if(numContainer->size<2)
							{
								return WRONG_FORM;
							}
							else
							{
								b=(float*)popOut(numContainer);
								a=(float*)popOut(numContainer);
								if(flag=(singleCal(*a,*b,*op, singleRes))==GOOD_RESULT)
								{
									
									pushIn(numContainer,singleRes);
									pushIn(symContainer,&source[i]);
								}
								else
								{
									return flag;
								}
							}
						}
						else
						{
							pushIn(symContainer,op);
							pushIn(symContainer, &source[i]);
						}
					}
				}
				break;
			case '*':
			case '/':
				if(symContainer->size==0)
				{
					pushIn(symContainer, &source[i]);
				}
				else
				{
					op=(char*)popOut(symContainer);
					if(*op=='*'||*op=='/')
					{
						if(numContainer->size<2)
						{
							return WRONG_FORM;
						}
						else
						{
							b=(float*)popOut(numContainer);
							a=(float*)popOut(numContainer);
							if(flag=(singleCal(*a,*b,*op, singleRes))==GOOD_RESULT)
							{
								pushIn(numContainer,singleRes);
								pushIn(symContainer,&source[i]);
							}
							else
							{
								return flag;
							}
						}
					}
					else
					{
						pushIn(symContainer,op);
						pushIn(symContainer, &source[i]);
					}
				}
				break;
			case '}':
				if(symContainer->size==0)
				{
					return WRONG_FORM;
				}
				op=(char*)popOut(symContainer);
				while(*op!='{')
				{
					if(numContainer->size<2)
					{
						return WRONG_FORM;
					}
					else
					{
						b=(float*)popOut(numContainer);
						a=(float*)popOut(numContainer);
						
						if(flag=(singleCal(*a,*b,*op, singleRes))==GOOD_RESULT)
						{
							
							pushIn(numContainer,singleRes);
							
						}
						else
						{
							return flag;
						}
						op=(char*)popOut(symContainer);
					}
				}
				break;
			default:
				return BAD_INPUT;
			}
		}
	}

	while(symContainer->size>0)
	{
		op=(char*)popOut(symContainer);
		
		if(numContainer->size<2)
		{
			return WRONG_FORM;
		}
		else
		{
			b=(float*)popOut(numContainer);
			a=(float*)popOut(numContainer);
			
			if(flag=(singleCal(*a,*b,*op, singleRes))==GOOD_RESULT)
			{
				
				pushIn(numContainer,singleRes);
			}
			else
			{
				return flag;
			}
		}
	}
	if(numContainer->size>1)
	{
		return WRONG_FORM;
	}
	a=(float*)popOut(numContainer);
	*result=*a;
	return GOOD_RESULT;
}

int main(int argc, char *argv[])
{
	float *res;
	int flag;
	for(int i=1; i<argc; i++)
	{
		if((flag=calculate(argc, argv[i], res))==GOOD_RESULT)
		{
			printf(1,"%f\n", *res);
		}
		else
		{
			printf(1, "%d\n", flag);
		}
	}
	exit();
}