#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "dataLink.h"

typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } CommandState;
typedef enum { SET, UA_SENDER, UA_RECEIVER, DISC_SENDER, DISC_RECEIVER } CommandType;
typedef enum { RECEIVING_SET, RECEIVING_DATA, RECEIVING_DISC, RECEIVING_UA, RECEIVING_DONE }ReceiverState;


const unsigned int COMMAND_LENGTH = 5;
const unsigned char SET_FRAME[] = {FLAG, A_SENDER, C_SET, A_SENDER^C_SET, FLAG};
const unsigned char UA_SENDER_FRAME[] = {FLAG, A_SENDER, C_UA, A_SENDER^C_UA, FLAG};
const unsigned char UA_RECEIVER_FRAME[] = {FLAG, A_RECEIVER, C_UA, A_RECEIVER^C_UA, FLAG};
const unsigned char DISC_SENDER_FRAME[] = {FLAG, A_SENDER, C_DISC, A_SENDER^C_DISC, FLAG};
const unsigned char DISC_RECEIVER_FRAME[] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER^C_DISC, FLAG};

int timeout = FALSE;
int numTransmissions = 0;

void handleTimeout(){
	timeout = TRUE;
}

void setNextAlarm(){
	timeout = FALSE;
	numTransmissions++;
	alarm(linkInfo.timeout);
}

void resetAlarm(){
	numTransmissions = 0;
	timeout = FALSE;
}


int transmitterOpenProtocol(int fd);
int receiverOpenProtocol(int fd);
int receiverCloseProtocol(int fd);
int transmitterCloseProtocol(int fd);
int readFrame(int fd, char *frame, int status);
int receiverStateManager(int fd);

int checkCommand(char *frameReceived, const unsigned char *frameExpected){
	if (sizeof(frameReceived) < COMMAND_LENGTH)
		return -1;

	int i = 0;
	for(i = 0; i < COMMAND_LENGTH; i++){
		if (frameReceived[i] != frameExpected[i])
			return -2;
	}
	return 0;
}

int openProtocol(struct applicationLayer app){
  linkInfo.baudRate = BAUDRATE;
  linkInfo.sequenceNumber = 0;
  linkInfo.timeout = TIMEOUT;
  linkInfo.numTransmissions = N_TRIES;

  if (app.status == TRANSMITTER){
	(void) signal(SIGALRM, handleTimeout);
    return transmitterOpenProtocol(app.fileDescriptor);
  }else if (app.status == RECEIVER){
    return receiverStateManager(app.fileDescriptor);
  } else {
    return -1;
  }
}

int transmitterOpenProtocol(int fd){
	resetAlarm();
  do{
printf("WRITING SET\n");
  write(fd,SET_FRAME,COMMAND_LENGTH);
	setNextAlarm();
  printf("WAITING UA\n");
  }while(readFrame(fd, linkInfo.frame, TRANSMITTER) < 0 && numTransmissions <= linkInfo.numTransmissions);
	
	if (numTransmissions > linkInfo.numTransmissions){
		printf("The connection with receiver could not be established.\n");
		return -1;
	} else {
		if (checkCommand(linkInfo.frame, UA_RECEIVER_FRAME) == 0){
  			//printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
  		return 0;
		} else {
		printf("Couldn't receive UA\n");
		return -1;
		}
	}
}

int receiverOpenProtocol(int fd){
  printf("WAITING SET\n");
  do{
	readFrame(fd, linkInfo.frame, RECEIVER);
  	printf("WRITING UA\n");
  	write(fd,UA_RECEIVER_FRAME,COMMAND_LENGTH);
}while (checkCommand(linkInfo.frame, SET_FRAME) == 0);
  return 0;
}

int dataWrite(char *packet){
  return 0;
}

int dataRead(char *packet){
  return 0;
}

int closeProtocol(struct applicationLayer app){
  if (app.status == TRANSMITTER){
    return transmitterCloseProtocol(app.fileDescriptor);
  }else if (app.status == RECEIVER){
    return receiverCloseProtocol(app.fileDescriptor);
  } else {
    return -1;
  }
  return 0;
}

int transmitterCloseProtocol(int fd){
 resetAlarm();
  do{
printf("WRITING DISC\n");
  write(fd,DISC_SENDER_FRAME,COMMAND_LENGTH);
	setNextAlarm();
  printf("WAITING DISC\n");
  }while(readFrame(fd, linkInfo.frame, TRANSMITTER) < 0 && numTransmissions <= linkInfo.numTransmissions);
	
	if (numTransmissions > linkInfo.numTransmissions){
		printf("The connection with receiver could not be established.\n");
		return -1;
	} else {
		if (checkCommand(linkInfo.frame, DISC_RECEIVER_FRAME) == 0){
  			//printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
			printf("WRITING UA");
  			write(fd,UA_SENDER_FRAME,COMMAND_LENGTH);
  		return 0;
		} else {
		printf("Couldn't receive DISC\n");
		return -1;
		}
	}
  return 0;
}

int receiverCloseProtocol(int fd){
  printf("WAITING DISC");
  readFrame(fd, linkInfo.frame, RECEIVER);
  printf("WRITING DISC");
  write(fd,DISC_RECEIVER_FRAME,COMMAND_LENGTH);
  printf("WAITING UA");
  readFrame(fd, linkInfo.frame, RECEIVER);
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
                        if ((status == RECEIVER && byte == A_SENDER) || (status == TRANSMITTER && byte == A_RECEIVER)) {
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

		if (timeout == TRUE ) return -1;
        }

        return i;
}

int receiverStateManager(int fd) {
	int triedOnce = FALSE;

	ReceiverState receiverState = RECEIVING_SET;

		
        while (receiverState != RECEIVING_DONE) {
				printf("chegeiii\n");
				printf("%d\n",receiverState);
                switch(receiverState) {
                case RECEIVING_SET:
printf("do belo");
				   readFrame(fd, linkInfo.frame, RECEIVER);
				if (checkCommand(linkInfo.frame, SET_FRAME) == 0){
		  			printf("WRITING UA\n");
		  			write(fd,UA_RECEIVER_FRAME,COMMAND_LENGTH);
					triedOnce = TRUE;
				} else if (triedOnce == TRUE){
					triedOnce = FALSE;
					receiverState = RECEIVING_DATA;
					printf("mudei\n");
				}
			break;

                case RECEIVING_DATA:
printf("do belo");
					if (checkCommand(linkInfo.frame, DISC_SENDER_FRAME) == 0){
                        receiverState = RECEIVING_DISC;
						}
			//LLREAD
                        break;

                case RECEIVING_DISC:
printf("nao mudei de estado lol");
printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
					if (checkCommand(linkInfo.frame, DISC_SENDER_FRAME) == 0){
		  				printf("WRITING DISC\n");
		  				write(fd,DISC_RECEIVER_FRAME,COMMAND_LENGTH);
						printf("WRITED DISC\n");
				   		readFrame(fd, linkInfo.frame, RECEIVER);
printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
					}
printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
					if (checkCommand(linkInfo.frame, UA_SENDER_FRAME) == 0){
						receiverState = RECEIVING_UA;
printf("%x:%x:%x:%x:%x\n", linkInfo.frame[0], linkInfo.frame[1], linkInfo.frame[2], linkInfo.frame[3], linkInfo.frame[4]);
					}
					break;

                case RECEIVING_UA:
return 0;
printf("do belo");
						receiverState = RECEIVING_DONE;
printf("do belo");
                        break;

case RECEIVING_DONE:
printf("do belo1");
	break;

                default:
printf("do belo2");
                        break;
                }
        }

        return 0;
}
