
/*********************************************************************               
                          CLIENT PROGRAM  
This program requests the service of a server and communicates with it.
To execute the program enter <name> <name of server> <port number>.          
Insert '!'before usual commands.
'ds' command gives the disc space.
"!exit"command exits from the program. 
*********************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char MSG[]="%%send";    /* send command declaration */  
char NEST[]="!exit";     /* exit command declaration */
int    clr,sock,a,mlen,getline(int);
char   c,*tmp,DATA[1000],EXP[1000],CLR[]="!clear";
struct hostent *hp,*gethostbyname();
struct sockaddr_in server;

/*********************************************************************
  Interactive mode
**********************************************************************/
int get_and_do(void) {
    tmp=DATA; 
    while(1){ 
      clr=1;                
      printf("[command]# ");
      gets(EXP);                   /* reading command */
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
      a=write(sock,DATA, mlen+1);
      if(a<0)perror("can't write on stream socket");
      else getline(sock);              /* function to print data */
      memset(DATA,0,mlen+1);
      memset(EXP,0,mlen+1);
    }    
        
  return 1;
}
/********************************************************************
  executing single command
**********************************************************************/
int single_command (char **command) {
  int i=0;
  strcpy(DATA,command[i]);
  i++;
  while(command[i] != NULL) {
   strcat(DATA," ");
   strcat(DATA,command[i]);
   i++;
  }
  a=write(sock,DATA, strlen(DATA)+1);
  if(a<0)perror("can't write on stream socket");
  else getline(sock);              /* function to print data */
  return 1;
}
/***********************************************************************
                        RECIEVES THE DATA  
Recieves the socket descriptor as argument.
It displays data send by the server line by line.
Returns an integer.
***********************************************************************/

  int getline(int sock){
    char buff[1000],d;
    char *pt=NULL;
    int count=0,n=1,rvalue=1;
    while(1){
      count=0;
      while(1){
       if((rvalue=read(sock,buff+count,1))<=0)break; 
                                      /** reading a single character **/
       if(buff[count]=='\n')break;
       count++;
       if(count>999) count=999;
      }
      if(rvalue==0)break;
      buff[count]='\0';
      if(strcmp(buff,MSG)==0)break; /* comparison to check end of data */
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
int main ( int argc,char **argv){
  int sock;
  if( (strcmp(argv[3],"pd")==0) ||
      (strcmp(argv[3],"rs")==0) ||
      (strcmp(argv[3],"ad")==0) ) {
      User_id;
  }
  if (argc < 3) {
    printf("USAGE: client <host> <port>\n");
    exit(0);
  }
  if( (sock = open_and_connect(argv[1],atoi(argv[2]))) > 0 ) {
    if(argc > 3)  single_command(argv+3);
    else get_and_do();
    close(sock);
    return 0;
  }
  else return -1;
}
