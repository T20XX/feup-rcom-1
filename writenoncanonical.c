/*Non-Canonical Input Processing*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "application.h"
#include "utils.h"

int main(int argc, char** argv)
{
  if ( (argc < 7) ||
  ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort FilePath BaudRate FrameSize NumberOfTries Timeout\n\tex: nserial /dev/ttyS1 ./image.jpg B9600 256 3 5\n");
    exit(1);
  }

  printf("===============================================================\n");
  printf("=                       FILE SENDER                           =\n");
  printf("=                          RCOM                               =\n");
  printf("=                       Transmitter                           =\n");
  printf("=     António Melo & Margarida Viterbo & Telmo Barros         =\n");
  printf("===============================================================\n");
  sleep(1);

  if (llopen(argv[1], TRANSMITTER, atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6])) < 0){
    perror("llopen");
    //exit(3);
  }

  if (llwrite(argv[2]) < 0){
    perror("llwrite");
    //exit(4);
  }

  if (llclose() < 0){
    perror("llclose");
    exit(5);
  }


  return 0;
}
