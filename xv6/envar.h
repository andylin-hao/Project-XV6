#define NAME_LIMIT        30
#define PATH_LIMIT        30
#define CONTENT_LIMIT     1000
#define VAR_LIMIT		  100

struct enVariable
{
	char name[NAME_LIMIT];
	char value[CONTENT_LIMIT];
	int nameLength;
	int size;
};

struct enVars
{
	struct enVariable contents[VAR_LIMIT];
	int varNum;
};

extern struct enVars enVarContainer;
extern int initiated;