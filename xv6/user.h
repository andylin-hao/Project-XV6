struct stat;

#define CMDTYPE_TEDIT     0
#define CMDTYPE_CONSOLE   1
#define SET_CMDTYPE       1

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int hlight(int);
int swpath(void);
int retprt(void);
int cmdmod(int,int);
int clrscr();
int getpos();
int setcln(int,char*);
int lstpos();
int hlline(int);
int delhlt(int);
int alarm(int ticks, void (*handler)());
int time(void);
int outexac(int x, int y, char word);
int kbevent(void (*keyboardHandler)(int (*getc)(void)));
int gameget(void);

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
