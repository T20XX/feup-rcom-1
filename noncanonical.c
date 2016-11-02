/*Non-Canonical Input Processing*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "application.h"

int main(int argc, char** argv){
  if ( (argc < 6) ||
  ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort BaudRate FrameSize NumberOfTries Timeout\n\tex: nserial /dev/ttyS1 B9600 256 3 5\n");
    exit(1);
  }

  printf("===============================================================\n");
  printf("=                       FILE SENDER                           =\n");
  printf("=                          RCOM                               =\n");
  printf("=                        Receiver                             =\n");
  printf("=     António Melo & Margarida Viterbo & Telmo Barros         =\n");
  printf("===============================================================\n");
  sleep(1);


  if (llopen(argv[1], RECEIVER, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5])) < 0){
    perror("llopen");
    exit(3);
  }

  // if (llread() < 0){
  //   perror("llread");
  // 	exit(4);
  // }

  if (llclose() < 0){
    perror("llclose");
    exit(5);
  }

  // msg[0] = 0;
  //   while (STOP==FALSE) {       /* loop for input */
  //     res = read(fd,buf,1);   /* returns after 1 char have been input */
  //     buf[res]=0;               /* so we can printf... */
  // printf("%s", buf, res);
  // strcat(msg,buf);
  //   if (buf[0]=='\0'){
  // 	printf("\n", buf, res);
  // int tam = strlen(msg)+1;
  // int n;
  // n = write(fd,msg,tam);
  // 	STOP=TRUE;
  // }
  //   }
  return 0;
}
