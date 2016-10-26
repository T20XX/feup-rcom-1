/*Non-Canonical Input Processing*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "application.h"
#include "utils.h"

int main(int argc, char** argv)
{
    if ( (argc < 3) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort FilePath\n\tex: nserial /dev/ttyS1 ./image.jpg\n");
      exit(1);
    }

    /*buf[25] = '\n';

    	gets(buf);
	int tam = strlen(buf)+1;
	int n;
	n = write(fd,buf,tam);*/

  if (llopen(argv[1], TRANSMITTER) < 0){
    perror("llopen");
		exit(3);
  }

  // if (llwrite(argv[2]) < 0){
  //   perror("llwrite");
	// 	exit(4);
  // }

  if (llclose() < 0){
    perror("llclose");
		exit(5);
  }


    return 0;
}
