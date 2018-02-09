#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int Jobid=-1;
FILE *jf;


/*********************************************************************               
                          CLIENT PROGRAM  
*********************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>

int clrscrn(char *clr);

char buff[10000];
char MSG[]="%%send";    /* send command declaration */  
char NEST[]="!exit";     /* exit command declaration */
int    clr,sock,a,mlen,pgetline(int);
char   c,*tmp,DATA[1000],EXP[1000],CLR[]="!clear";
struct hostent *hp,*gethostbyname();
struct sockaddr_in server;
#define TIMEOUT -3000
#define SPACE ' '
int read_sock(int sock,char *data,int count) {
	int ret;
	struct pollfd pfd;

	pfd.fd = sock;
	pfd.events = POLLIN;
	pfd.revents = 0;

	/*Poll for data from the node*/
	ret = poll(&pfd,1,TIMEOUT);
	if(ret == -1) { perror("poll timeout"); }
        else {
          ret =0;
	  if(pfd.revents & POLLIN) { ret = read(sock,data,count); }
        }

	return ret;
}

int write_sock(int sock,char *data,int count) {
	int ret;
	struct pollfd pfd;

	pfd.fd = sock;
	pfd.events = POLLOUT|POLLRDHUP;
	pfd.revents = 0;

	/*Poll for data from the node*/
	ret = poll(&pfd,1,TIMEOUT);
	if(ret == -1) { perror("poll timeout"); }
        else {
          ret =0;
	  if(pfd.revents & POLLOUT) { ret = write(sock,data,count); }
          /* Other checks possible....*/
        }

	return ret;
}

/********************************************************************
  executing single command
**********************************************************************/
int single_command_exec (char **command) {
  int i=0;
  strcpy(DATA,command[i]);
  i++;
  while(command[i] != NULL) {
   strcat(DATA," ");
   strcat(DATA,command[i]);
   i++;
  }
  a=write_sock(sock,DATA, strlen(DATA)+1);
  if(a<0)perror("can't write on stream socket");
  else pgetline(sock);              /* function to print data */
  return 1;
}
/***********************************************************************
                        RECIEVES THE DATA  
Recieves the socket descriptor as argument.
It displays data send by the server line by line.
Returns an integer.
***********************************************************************/

  int pgetline(int sock){
    char d;
    char *pt=NULL;
    char buf[10000];
    int count=0,n=1,rvalue=1;
    buff[0]='\0';
    while(1){
      count=0;
      while(1){
       if((rvalue=read_sock(sock,buf+count,1))<=0)break; 
                                      /** reading a single character **/
       if(buf[count]=='\n')break;
       count++;
       if(count>9990) count=9990;
      }
      if(rvalue==0) {
            break;
      }
      buf[count]='\0';
      if(strcmp(buf,MSG)==0) {
         break; /* comparison to check end of data */
      }
      else strcpy(buff,buf);
//      puts(buff);                   /* printing the output */
      memset(buf,0,rvalue);
    }
    return(0);
    
  }
int open_and_connect(char *host,int port) {
  sock = socket(AF_INET,SOCK_STREAM,0);   /* socket opening */
  if( sock <0) {                           
   perror("Error in opening stream socket...   ");
   return -1;
  } 
  server.sin_family = AF_INET;
  hp = gethostbyname(host);
  if(hp==0) {
   fprintf(stderr,"%s :unknown host\n",host);
   return -1;
  }
  memcpy((char *)&server.sin_addr,(char *)hp->h_addr,hp->h_length);
  server.sin_port = htons(port);
  if( connect(sock,(struct sockaddr *)&server,sizeof(server)) <0) {
   perror("Error in connecting stream socket...  ");
   return -1;
  }
  return sock;
}

/**********************************************************************
                            MAIN PROGRAM
It creates socket,requests service to a server, send commands and
 recieves the processed data from the server.
**********************************************************************/
#define User_id {\
    if(getuid()!= 0) {\
      printf("Only root can execute this command...\n");\
      exit(1);\
    }\
}
int single_command(char *machine,int port,char **command) {
  int sock,ret=-1;
  if( (sock = open_and_connect(machine,port)) > 0 ) {
      single_command_exec(command);
      close(sock);
      ret =0;
  }
  return ret;
}

int wordcount(char *buf){
    int count=1;
    char *tmp;

    tmp=buf;
    while(1){
      while((*tmp!=SPACE)&&(*tmp!='\0')){
       tmp++;
      }
      if(*tmp=='\0')break;
      count++;
      tmp++;
    }
    return count;
}

/*******************************************************************************
                SPLITTING THE COMMAND INTO ARGUMENTS
Recieves the command and number of arguments.
Splits the command into arguments.
Returns the pointer to array of pointers to the arguments.
*******************************************************************************/

char **getwords(char *buf){
    char *pt,**ptr;
    int count1=0,count;

    count=wordcount(buf);
    count1=strlen(buf);
    pt = (char *)malloc(sizeof(char)*(count1+1));
    ptr=(char **)malloc(sizeof(char *)*(count+1));
    strcpy(pt,buf);
    while(1){
      *ptr=pt;
      while((*pt!=SPACE)&&(*pt!='\0')){
         pt++;
      }
      if(*pt=='\0')break;
      *pt='\0';
      pt++;
      ptr++;
    }
    ptr++;
    *ptr=NULL;
    ptr=ptr-count; 
    return (ptr);  
}  
int report_jobend(int jobid,int uid,int sid,int status){
  int nmch=0,i,gmach=0,nswon,j,ok=1,loop=0,avmach=0;
  char buf[10000];
  char *crmserver;
  char command[200];
  int  *machine;
  FILE *pp,*fp;
  char **args,**words;

  crmserver = getenv("CRMSERVER");
  if(crmserver==NULL) {
     fprintf(stderr,"Error: CRMSERVER not set..\n");
//     return -1;
     crmserver= (char *)malloc(100);
     strcpy(crmserver,"172.30.101.10");
  }
  fprintf(stderr,"CRMSERVER: %s\n",crmserver);
  args = (char **)malloc(sizeof(char *)*6);
  args[0]= (char *) malloc(3);
  strcpy(args[0],"je"); 
  args[1]= (char *) malloc(10);
  sprintf(args[1],"%d",jobid);
  args[2]= (char *) malloc(10);
  sprintf(args[2],"%d",uid);
  args[3]= (char *) malloc(10);
  sprintf(args[3],"%d",sid);
  args[4]= (char *) malloc(10);
  sprintf(args[4],"%d",status);
  args[5]= NULL;
  single_command(crmserver,20350,args);
  free(args[3]);
  free(args[2]);
  free(args[1]);
  free(args[0]);
  free(args);
  fp = fopen("OVER", "w");
  fprintf(fp,"%-d  Executed \n",jobid);
  fclose(fp);
  return 1;
}
int GoBackGround(int argc,char **argv) {
  char hostname[100],sidstr[30];
  int status,pid,pipes[2];;
  pipe(pipes);
  if( (pid=fork()) ==0 ) {
    int status;
    close(pipes[0]);
    chdir(argv[3]);
    setgid(atoi(argv[2]));
    setuid(atoi(argv[1]));
    close(2);
    open(".ErrFile",O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
    if(fork()!=0) return 1;
    if( (pid=fork()) !=0 ) {
     gethostname(hostname,99);
     sprintf(sidstr,"%d",pid);
     write(pipes[1],sidstr,30);
     waitpid(pid,&status,0);
     if(WIFEXITED(status)) report_jobend(Jobid,atoi(argv[1]),pid,1);
     else report_jobend(Jobid,atoi(argv[1]),pid,0);
     remove(".Jobid");
     return 1;
    }
    else {
     int sid;
     setgid(atoi(argv[2]));
     setuid(atoi(argv[1]));
     sid = setsid();
//     printf("SID= %d\n",sid);
     chdir(argv[3]);
     close(1);
     open("nohup.out",O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
     close(2);
     dup(1);
     execvp(argv[5],argv+5);
     fprintf(stderr,"Failed to execute %s\n",argv[5]);
     return 1;
    }
  }
  else {
    close(pipes[1]);
    read(pipes[0],sidstr,30);
    printf("%d %s\n",Jobid,sidstr);
    waitpid(pid,&status,0);
    return 1;
  }
}
int main(int argc,char **argv){
  int uid;
  pid_t pid;
  char JobFile[200];
  if(argc <3 ) {
    fprintf(stderr,"Usage: %s <uid> <gid> <word_directory> <no of args > <program> [<args...>]\n",argv[0]);
    _exit(1);
  }
  sprintf(JobFile,"%-s/.Jobid",argv[3]);
//  printf("%s\n",JobFile);
  jf = fopen(JobFile,"r");
  if(jf==NULL) {
       fprintf(stderr,"Filed to get Jobid\n");
       printf("FAILED\n");
       return 0;
  }
  fscanf(jf,"%d",&Jobid);
  fclose(jf);
  GoBackGround(argc,argv);
  return 1;
}
