#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#define UNDEF -2123456789
#define EOL   -2123456780
#define EOA   -2123456781

typedef void(*VFP)();

char op[10], arg[10240];
char*Code[1024]={0};
long IP=0, FP=50, SP=100;
long Data[1024];

struct __vec__ { long*arr, size, fill; } v;

void add(){ Data[SP++] = Data[FP-4] + Data[FP-5]; }
void sub(){ Data[SP++] = Data[FP-4] - Data[FP-5]; }

void println(){
  Data[SP++] = printf((char*)Data[FP-4],
		      Data[FP-5], Data[FP-6], Data[FP-7],
		      Data[FP-8], Data[FP-9], Data[FP-10]);
  puts("");
}

void list(){
  struct __vec__ *v = malloc(sizeof(struct __vec__));
  long n, sz;
  for(n=5; Data[FP-n]!=UNDEF; n++);
  sz = n-5;
  v->arr = malloc((sz+1)*sizeof(long));
  v->size = sz;
  v->fill = sz;
  for(n=5; Data[FP-n]!=UNDEF; n++)
    v->arr[n-5] = Data[FP-n];
  v->arr[sz] = UNDEF;
  Data[SP++] = (long)v;
}

struct __native__ {
  char*s; VFP f;
} Symtab[] = {
  {"sub",sub},
  {"add",add},
  {"println",println},
  {0,0}};

void dump (){
  printf("*== IP=%2ld / FP=%2ld / SP=%2ld\n", IP, FP, SP);
  printf("|   Code: %s", Code[IP]);
  printf("+-- Data: ");
  for(long n=100; n<SP; n++)
    printf(" %ld", Data[n]);
  puts("");
}
void ret (){
  long rv = Data[SP-1];
  SP = FP;
  IP = Data[--SP];
  FP = Data[--SP];
  SP = Data[SP-1];
  Data[SP++] = rv;
}
void step (){
  dump();
  sscanf(Code[IP++], "%s %s\n", op, arg);
  printf("[%s][%s]\n", op, arg);
  switch(*op){
  case 'Q':{
    long n = atoi(arg);
    if(n||!strcmp(arg,"0")) Data[SP++] = n;
    else if(arg[0]=='$')    Data[SP++] = (long)strdup(arg+1);
    else { printf("ERROR1\n"), exit(1);}
    break;}
  case 'L':{
    long n = atoi(arg);
    if(n||!strcmp(arg,"0")) { Data[SP++] = n; break; }
    else if(arg[0]=='$')    { printf("ERROR3\n"), exit(1);}
    else {
      for(struct __native__ *record = Symtab;record->s;record++)
	if(!strcmp(arg,record->s)){
	  Data[SP++] = (long)record->f;
	  return;}
      printf("ERROR5[%s not found]\n",arg), exit(1);}}
  case 'G':    IP  = atoi(arg);    break;
  case 'J':    IP += atoi(arg);    break;
  case 'C':{
    long n = atoi(arg);
    long fn = Data[SP-1];
    Data[SP-1] = SP+n;
    Data[SP++] = FP;
    Data[SP++] = IP;
    FP = SP;
    if(fn<0x1000){ IP=fn; break;}
    ((VFP)(void*)fn)();
    dump(); printf("[R][<synthetic>]\n");}
  case 'R': ret(); break;
  case 'X': printf("EXIT!\n"), exit(0);
  case 'Y':{long n = atoi(arg); SP -= n; break;}
  default: printf("ERROR2[%c]\n",*op), exit(1);}}

int main (){
  char buf[10240];
  long n=0;
  memset(Code, 0, sizeof(Code));
  while(fgets(buf,10240,stdin)) Code[n++] = strdup(buf);
  Code[n] = 0;
  while(1){ step(); usleep(100000); }
  dump();}
