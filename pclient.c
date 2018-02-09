
/*********************************************************************               
                          CLIENT PROGRAM  
This program requests the service of a server and communicates with it.
To execute the program enter <name> <name of server> <port number>.          
Insert '!'before usual commands.
'ds' command gives the disc space.
"!exit"command exits from the program. 
*********************************************************************/

#define _GNU_SOURCE
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

char MSG[]="%%send";    /* send command declaration */  
char NEST[]="!exit";     /* exit command declaration */
int    clr,sock,a,mlen,pgetline(int);
char   c,*tmp,DATA[1000],EXP[1000],CLR[]="!clear";
struct hostent *hp,*gethostbyname();
struct sockaddr_in server;
#define TIMEOUT -3000

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

/*********************************************************************
  Interactive mode
**********************************************************************/
int get_and_do(void) {
    tmp=DATA; 
    while(1){ 
      clr=1;                
      printf("[command]# ");
      fgets(EXP,900,stdin);                   /* reading command */
      EXP[strlen(EXP)-1]='\0';
      if(strlen(EXP)==0)continue;  /* no command */
      if(strcmp(EXP,CLR)==0){
        clrscrn(EXP+1);           /* function to clear screen */
        clr=0;
      }
      if(clr==0)continue;
      c=EXP[sizeof (NEST)];    
      EXP[sizeof (NEST)]='\0';
      if((strcmp(EXP,NEST))==0){        /* comparison to exit */
       printf ("ending connection...\n");
       break;
      }   
      EXP[sizeof (NEST)]=c;
      mlen=strlen(EXP);
      strcpy(tmp,EXP); 
      a=write_sock(sock,DATA, mlen+1);
      if(a<0){
        //  printf("can't write on stream socket\n");
          perror("can't write on stream socket");
      }
      else  {
        // printf(" a> 0 getting line\n");
        pgetline(sock);              /* function to print data */
      }
      memset(DATA,0,mlen+1);
      memset(EXP,0,mlen+1);
    }    
        
  return 1;
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
    char buff[1000],d;
    char *pt=NULL;
    int count=0,n=1,rvalue=1;
    while(1){
      count=0;
      while(1){
       if((rvalue=read_sock(sock,buff+count,1))<=0)break; 
                                      /** reading a single character **/
       if(buff[count]=='\n')break;
       count++;
       if(count>999) count=999;
      }
      if(rvalue==0) {
            break;
      }
      buff[count]='\0';
      if(strcmp(buff,MSG)==0) {
         break; /* comparison to check end of data */
      }
      puts(buff);                   /* printing the output */
      memset(buff,0,rvalue);
    }
    return(0);
    
  }
/***********************************************************************
                         CLEARS THE SCREEN
Recieves clear command as argument and execute it.
Returns an integer.
***********************************************************************/
  int clrscrn(char *clr){        
      int pid,status;
      pid=fork();
      if(pid==0){
       execlp(clr,clr,(char *)0);
       exit(127);
      }
      pid=waitpid(pid,&status,0);
      return 0;
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
int multiple_command(char *machine,int port) {
  int sock,ret=-1;
  if( (sock = open_and_connect(machine,port)) > 0 ) {
      get_and_do();
      close(sock);
      ret =0;
  }
  return ret;
}
int main ( int argc,char **argv){
  int sock,ret =0;
  if(argc > 3) {
    if( (strcmp(argv[3],"pd")==0) ||
        (strcmp(argv[3],"rs")==0) ||
        (strcmp(argv[3],"ad")==0) ) {
        User_id;
    }
  }
  else User_id;
  if (argc < 3) {
    printf("USAGE: pclient <host> <port>\n");
    exit(0);
  }
  if(argc > 3)  ret = single_command(argv[1],atoi(argv[2]),argv+3);
  else          ret = multiple_command(argv[1],atoi(argv[2]));
  return ret;
}
