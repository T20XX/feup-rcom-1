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
int readFrame(int fd, char *frame, int status);

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
  printf("vou escrever");
	write(fd,SET_FRAME,sizeof(SET_FRAME));
  printf("vou ler");
  readFrame(fd, linkInfo.frame, TRANSMITTER);
  printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
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
  printf("vou ler");
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
  readFrame(fd, linkInfo.frame, RECEIVER);
  printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
//sleep(5);
  printf("vou escrever");
  write(fd,UA_RECEIVER_FRAME,sizeof(UA_RECEIVER_FRAME));
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

int readFrame(int fd, char *frame, int status) {
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
                        if (status == RECEIVER && byte == A_SENDER || status == TRANSMITTER byte == A_RECEIVER) {
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
