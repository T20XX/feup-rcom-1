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

int packetSequenceNumber = 0;
int imageDescriptor;

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
  int n = 0;

  if ( (id = open (file , O_RDONLY)) < 0){
    perror(file);
    exit(2);
  }

  //SIZE
  stat(file, &fileInfo);
  //printf("%ld\n", fileInfo.st_size);
  //printf("%ld\n", sizeof(fileInfo.st_mode));

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

  //printf("%s\n", per);

  startPacket = malloc(strlen(file) + 4 + 10 + 7 + 1);
  //char *pointer = startPacket;
  startPacket[n++] = 2;
  startPacket[n++] = T_SIZE;
  startPacket[n++] = 4;
  //memcpy(startPacket[3], &fileInfo.st_size, sizeof(fileInfo.st_size));
  startPacket[n++] = (fileInfo.st_size & 0xFF000000) >> 24;
  startPacket[n++] = (fileInfo.st_size & 0x00FF0000) >> 16;
  startPacket[n++] = (fileInfo.st_size & 0x0000FF00) >> 8;
  startPacket[n++] = (fileInfo.st_size & 0x000000FF);
  //printf("%x:%x:%x:%x\n", startPacket[3], startPacket[4], startPacket[5], startPacket[6] & 0xff);
  //printf("%d\n", fileInfo.st_size);
  startPacket[n++] = T_NAME;
  startPacket[n++] = strlen(file);
  memcpy(&startPacket[n], file, strlen(file));
  printf("Sending startPacket...\n");
  if (dataWrite(app.fileDescriptor, startPacket, sizeof(file) + 4 + 10 + 7 + 1) != 0){
    return -1;
  }

  int bytesSent = 0;
  int bytesToSent = 0;
  char *packet;
  //fseek(id,0,SEEK_SET);

  while(bytesSent < fileInfo.st_size){
    if(fileInfo.st_size - bytesSent > DATA_MAX_SIZE){
      bytesToSent = DATA_MAX_SIZE;
    } else {
      bytesToSent = fileInfo.st_size - bytesSent;
    }
    packet = (char *) malloc(bytesToSent + 4);
    packet[0] = 1;
    //printf ("\n\n\n\n\n\n\n%d\n\n\n\n\n\n\n\n\n\n", packetSequenceNumber);
    packet[1] = packetSequenceNumber;
    packet[2] = ((bytesToSent) & 0xFF00) >> 8;
    packet[3] = (bytesToSent) & 0xFF;

    if (read(id,&packet[4],bytesToSent) == bytesToSent){
      //int i = 0;
    // for(i = 0; i< sizeof(packet); i++){
    //   printf("%x:",packet[i]);
    // }
    printf("Sending packet %d\n",packetSequenceNumber);
      if (dataWrite(app.fileDescriptor, packet, bytesToSent + 4) == 0){
        bytesSent += bytesToSent;
        packetSequenceNumber++;
      } else {
        return -1;
      }
    }
      free(packet);
  }
  char endPacket[] = {3};
  printf("Sending endPacket\n");
  if (dataWrite(app.fileDescriptor, endPacket, 1) != 0){
    return -1;
  }

  return 0;

}

int llread(char *packet, int length){
  int n = 0;
  //printf("%x\n",packet[0]);
  switch(packet[n++]){
    case 1:
    //printf("%x\n",packet[n]);
      if (packet[n++] == packetSequenceNumber){
          int size;
          size = (unsigned char)packet[n] * 256 + (unsigned char)packet[n+1];
          n+=2;
          printf("Writing packet in fileDescriptor\n");
          //printf("%d\n",size);
          write(imageDescriptor,&packet[n],size);

        packetSequenceNumber++;
      }
      break;
    case 2:
        if (packet[n] == T_SIZE){
          n++;
          n += packet[n];
          n++;
          if (packet[n] == T_NAME){
            n++;
            int filenameSize = packet[n];
            n++;
            char filename[256];
            memcpy(&filename, &packet[n], filenameSize);
            //printf("%s\n",filename);
            printf("Opening fileDescriptor\n");
            imageDescriptor = open(filename, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC);
          }
        }
      break;
    case 3:
        printf("Closing fileDescriptor\n");
        close(imageDescriptor);
      break;
    default:
      break;
  }
  return 0;
}

int llclose(){
  printf("Closing Protocol...\n");
  closeProtocol(app);

  if ( tcsetattr(app.fileDescriptor,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(app.fileDescriptor);
  return 1;
}
