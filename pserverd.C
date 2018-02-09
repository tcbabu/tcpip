#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define TRUE 1
int main(void) {

  daemon(0,1);
  do {
    system("killall -9 pserver");
    system("killall -9 jobserver");
    system("/sbin/pserver");
    system("/sbin/jobserver");
    sleep(36000);
  } while(TRUE);
  exit(0);
}   
 
