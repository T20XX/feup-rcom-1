#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include "application.h"
#include "dataLink.h"
#include "utils.h"

struct termios oldtio,newtio;

int llopen(const char *port, int status){
  int fd;

  /*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = open(port, O_RDWR | O_NOCTTY );
  if (fd <0) {perror(port); exit(-1); }

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
	if (status == TRANSMITTER){
  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */
	} else if (status == RECEIVER){
	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
	}

  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es)
  */
  tcflush(fd, TCIOFLUSH);
  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  app.fileDescriptor = fd;
  app.status = status;


  printf("New termios structure set\n");

  if (openProtocol(app) < 0){
    perror("Failed to establish connection");
    exit(-1);
  }

  return (fd);
}

int llwrite(const char *file){
  struct stat fileInfo;

  //struct dirent *dp;
  char per[10];
  char * startPacket;
  int id;

  if ( (id = open (file , O_RDONLY)) < 0){
    perror(file);
    exit(2);
  }

  //SIZE
  stat(file, &fileInfo);
  printf("%ld\n", fileInfo.st_size);
  printf("%ld\n", sizeof(fileInfo.st_mode));

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

  startPacket = malloc(sizeof(file) + 4 + 10 + 7 + 1);
  //char *pointer = startPacket;
  startPacket[0] = 2;
  startPacket[1] = T_SIZE;
  startPacket[2] = 4;
  //memcpy(startPacket[3], &fileInfo.st_size, sizeof(fileInfo.st_size));
  startPacket[3] = (fileInfo.st_size & 0xFF000000) >> 24;
  startPacket[4] = (fileInfo.st_size & 0x00FF0000) >> 16;
  startPacket[5] = (fileInfo.st_size & 0x0000FF00) >> 8;
  startPacket[6] = (fileInfo.st_size & 0x000000FF);
  printf("%x:%x:%x:%x\n", startPacket[3], startPacket[4], startPacket[5], startPacket[6] & 0xff);
  printf("%lx\n", fileInfo.st_size);
  startPacket[7] = T_NAME;
  startPacket[8] = sizeof(file);
  memcpy(&startPacket[9], file, sizeof(file) + 1);

  return 0;

}

int llread(){
  return 0;
}

int llclose(){
  closeProtocol(app);

  if ( tcsetattr(app.fileDescriptor,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(app.fileDescriptor);
  return 1;
}
