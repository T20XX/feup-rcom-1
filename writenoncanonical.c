/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define T_SIZE 0
#define T_NAME 1
#define T_DATE 2
#define T_PERMISSIONS 3

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd, c, res, id;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
	struct stat fileInfo;

    if ( (argc < 3) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort FilePath\n\tex: nserial /dev/ttyS1 ./image.jpg\n");
      exit(1);
    }

	if ( (id = open (argv[2] , O_RDONLY)) < 0){
		perror(argv[2]);
		exit(2);
	}


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


	struct dirent *dp;
	char per[10];
	char * startPacket;

	//SIZE
	stat(argv[2], &fileInfo);
	printf("%d\n", fileInfo.st_size);
	printf("%d\n", sizeof(fileInfo.st_mode));

	//PERMISSIONS
	if ( fileInfo.st_mode & S_IRUSR ) per[0] = 'r';    /* 3 bits for user  */
	if ( fileInfo.st_mode & S_IWUSR ) per[1] = 'w';
	if ( fileInfo.st_mode & S_IXUSR ) per[2] = 'x';

	if ( fileInfo.st_mode & S_IRGRP ) per[3] = 'r';    /* 3 bits for group */
	if ( fileInfo.st_mode & S_IWGRP ) per[4] = 'w';
	if ( fileInfo.st_mode & S_IXGRP ) per[5] = 'x';

	if ( fileInfo.st_mode & S_IROTH ) per[6] = 'r';    /* 3 bits for other */
	if ( fileInfo.st_mode & S_IWOTH ) per[7] = 'w';
	if ( fileInfo.st_mode & S_IXOTH ) per[8] = 'x';

	printf("%s\n", per);

	startPacket = malloc(sizeof(argv[2]) + 4 + 10 + 7 + 1);
  char *pointer = startPacket;
	startPacket[0] = 2;
	startPacket[1] = T_SIZE;
	startPacket[2] = 4;
  //memcpy(startPacket[3], &fileInfo.st_size, sizeof(fileInfo.st_size));
	startPacket[3] = (fileInfo.st_size & 0xFF000000) >> 24;
	startPacket[4] = (fileInfo.st_size & 0x00FF0000) >> 16;
	startPacket[5] = (fileInfo.st_size & 0x0000FF00) >> 8;
	startPacket[6] = (fileInfo.st_size & 0x000000FF);
  printf("%x:%x:%x:%x\n", startPacket[3], startPacket[4], startPacket[5], startPacket[6] & 0xff);
  printf("%x\n", fileInfo.st_size);
	startPacket[7] = T_NAME;
	startPacket[8] = sizeof(argv[2]);
  memcpy(&startPacket[9], argv[2], sizeof(argv[2]) + 1);
	//startPacket[9 + sizeof(argv[2]) + 1] = 2;
	//startPacket[9 + sizeof(argv[2]) + 2] = 10;
	//startPacket[9 + sizeof(argv[2]) + 3] = per;

	/*fileSize = lseek(id, 0, SEEK_END);
	printf("%i\n",fileSize);
	lseek(id, 0, SEEK_SET);*/

    /*for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }*/

    /*testing*/
    buf[25] = '\n';

    	gets(buf);
	int tam = strlen(buf)+1;
	int n;
	n = write(fd,buf,tam);

	/*
	while(len > nw){
		n = write(fd,buf+nw,tam-nw);
		if(!n){
			break;
		}
	nw += n;
	}

	if(nw < tam){
	perror("ups");
	}*/



    //res = write(fd,buf,255);
    //printf("%d bytes written\n", nw);


  /*
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
    o indicado no gui�o
  */





    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}
