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
    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    if (llopen(argv[1], RECEIVER) < 0){
      perror("llopen");
  		exit(3);
    }

    if (llread() < 0){
      perror("llread");
  		exit(4);
    }

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
