#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "dataLink.h"

typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } CommandState;
typedef enum { SET, UA_SENDER, UA_RECEIVER, DISC_SENDER, DISC_RECEIVER } CommandType;


const unsigned int COMMAND_LENGTH = 5;
const unsigned char SET_FRAME[] = {FLAG, A_SENDER, C_SET, A_SENDER^C_SET, FLAG};
const unsigned char UA_SENDER_FRAME[] = {FLAG, A_SENDER, C_UA, A_SENDER^C_UA, FLAG};
const unsigned char UA_RECEIVER_FRAME[] = {FLAG, A_RECEIVER, C_UA, A_RECEIVER^C_UA, FLAG};
const unsigned char DISC_SENDER_FRAME[] = {FLAG, A_SENDER, C_DISC, A_SENDER^C_DISC, FLAG};
const unsigned char DISC_RECEIVER_FRAME[] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER^C_DISC, FLAG};


int transmitterOpenProtocol(int fd);
int receiverOpenProtocol(int fd);
int readFrame(int fd, char *frame);

int openProtocol(struct applicationLayer app){
  linkInfo.baudRate = BAUDRATE;
  linkInfo.sequenceNumber = 0;
  linkInfo.timeout = TIMEOUT;
  linkInfo.numTransmissions = N_TRIES;

  if (app.status == TRANSMITTER){
    return transmitterOpenProtocol(app.fileDescriptor);
  }else if (app.status == RECEIVER){
    return receiverOpenProtocol(app.fileDescriptor);
  } else {
    return -1;
  }
}

int transmitterOpenProtocol(int fd){
	write(fd,SET_FRAME,sizeof(SET_FRAME));
  readFrame(fd, linkInfo.frame);
  // olatile int STOP=FALSE;
  // char buf[255];
	// char msg[500];
  // int  res;
  //
  //
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

int receiverOpenProtocol(int fd){
  // volatile int STOP=FALSE;
  // char buf[255];
  // char msg[500];
  // int  res;
  //
  // msg[0] = 0;
  // while (STOP==FALSE) {       /* loop for input */
  //   res = read(fd,buf,1);   /* returns after 1 char have been input */
  //   buf[res]=0;               /* so we can printf... */
  //   printf("%s", buf, res);
  //   strcat(msg,buf);
  //   if (buf[0]=='\0'){
  //     printf("\n", buf, res);
  //     int tam = strlen(msg)+1;
  //     int n = write(fd,msg,tam);
  //     STOP=TRUE;
  //   }
  // }
  readFrame(fd, linkInfo.frame);
  write(fd,UA_RECEIVER_FRAME,sizeof(SET_FRAME));
  return 0;
}

int dataWrite(char *packet){
  return 0;
}

int dataRead(char *packet){
  return 0;
}

int closeProtocol(int status){
  return 0;
}

int readFrame(int fd, char *frame) {
        int i;
        unsigned char byte;
        CommandState state;

        i = 0;
        state = START_RCV;

        while (state != STOP_RCV) {
                read(fd, &byte, 1);

                switch(state) {
                case START_RCV:
                        if (byte == FLAG) {
                                frame[i++] = byte;
                                state = FLAG_RCV;
                        }
                        break;

                case FLAG_RCV:
                        if (byte == A_SENDER) {
                                frame[i++] = byte;
                                state = A_RCV;
                        }
                        else if (byte == FLAG) ;
                        else {
                                state = START_RCV;
                        }
                        break;

                case A_RCV:
                        if (byte != FLAG) {
                                frame[i++] = byte;
                                state = C_RCV;
                        }
                        else {
                                state = FLAG_RCV;
                        }
                        break;

                case C_RCV:
                        if (byte != FLAG) {
                                frame[i++] = byte;
                                state = BCC_OK;
                        }
                        else {
                                state = FLAG_RCV;
                        }
                        break;

                case BCC_OK:
                        if (byte == FLAG) {
                                frame[i++] = byte;
                                state = STOP_RCV;
                        }
                        else {
                                frame[i++] = byte;
                        }
                        break;

                default:
                        break;
                }
        }

        return i;
}
