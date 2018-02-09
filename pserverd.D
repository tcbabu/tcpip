#include <stdio.h>
#include <unistd.h>
#define TRUE 1
int main(void) {

  daemon(0,1);
  do {
    system("killall -9 pserver");
    system("pserver");
    sleep(86400);
  } while(TRUE);
  exit(0);
}   
 
