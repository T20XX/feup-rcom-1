#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "dataLink.h"

typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } CommandState;
typedef enum { SET, UA_SENDER, UA_RECEIVER, DISC_SENDER, DISC_RECEIVER } CommandType;
typedef enum { RECEIVING_SET, RECEIVING_DATA, RECEIVING_DISC, RECEIVING_UA, RECEIVING_DONE } ReceiverState;


const unsigned int COMMAND_LENGTH = 5;
const unsigned char SET_FRAME[] = {FLAG, A_SENDER, C_SET, A_SENDER^C_SET, FLAG};
const unsigned char UA_SENDER_FRAME[] = {FLAG, A_SENDER, C_UA, A_SENDER^C_UA, FLAG};
const unsigned char UA_RECEIVER_FRAME[] = {FLAG, A_RECEIVER, C_UA, A_RECEIVER^C_UA, FLAG};
const unsigned char DISC_SENDER_FRAME[] = {FLAG, A_SENDER, C_DISC, A_SENDER^C_DISC, FLAG};
const unsigned char DISC_RECEIVER_FRAME[] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER^C_DISC, FLAG};
const unsigned char RR_0_FRAME[] = {FLAG, A_RECEIVER, C_RR_0, A_RECEIVER^C_RR_0, FLAG};
const unsigned char RR_1_FRAME[] = {FLAG, A_RECEIVER, C_RR_1, A_RECEIVER^C_RR_1, FLAG};
const unsigned char REJ_0_FRAME[] = {FLAG, A_RECEIVER, C_REJ_0, A_RECEIVER^C_REJ_0, FLAG};
const unsigned char REJ_1_FRAME[] = {FLAG, A_RECEIVER, C_REJ_1, A_RECEIVER^C_REJ_1, FLAG};

int timeout = FALSE;
int numTransmissions = 0;

int n_i_frames = 0;
int n_timeout = 0;
int n_rej_frames = 0;

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
int transmitterCloseProtocol(int fd);
int readFrame(int fd, char *frame, int status);
int receiverStateManager(int fd);

int checkCommand(char *frameReceived, const unsigned char *frameExpected){
	if (sizeof(frameReceived) < COMMAND_LENGTH)
	return -1;

	int i = 0;
	for(i = 0; i < COMMAND_LENGTH; i++){
		if ((unsigned char)frameReceived[i] != (unsigned char)frameExpected[i])
		return -2;
	}
	return 0;
}

int openProtocol(struct applicationLayer app, int baudRate,int framesize, int noftries, int noftimeout){
	linkInfo.baudRate = baudRate;
	linkInfo.sequenceNumber = 0;
	linkInfo.timeout = noftimeout;
	linkInfo.numTransmissions = noftries;
	linkInfo.frame = malloc(framesize * 2);
	srand(time(NULL));
	printf("Opening Protocol...\n");
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
	char frameReceived[COMMAND_LENGTH];
	int commandIsOk;
	resetAlarm();
	tcflush(fd, TCIOFLUSH);
	printf("Sending SET\n");

	do{
		commandIsOk = 1;
		write(fd,SET_FRAME,COMMAND_LENGTH);
		setNextAlarm();

		readFrame(fd, frameReceived, TRANSMITTER);


		commandIsOk = checkCommand(frameReceived, UA_RECEIVER_FRAME);
	}while((commandIsOk != 0) && (numTransmissions < linkInfo.numTransmissions));

	if (numTransmissions >= linkInfo.numTransmissions || commandIsOk  < 0){
		printf("The connection with receiver could not be established.\n");
		return -1;
	}
	return 0;
}

int stuff(char *frame,unsigned char byte){
	if(byte == FLAG || byte == ESCAPE){
		*frame = ESCAPE;
		frame++;
		*frame = byte ^ STUFF_BYTE;
		return 0;
	}else{
		*frame = byte;
		return -1;
	}
}

int destuff(int length){
	int i=0,f;

	for(f= 0;f < length;f++,i++){
		if(linkInfo.frame[f] == ESCAPE){
			f++;
			linkInfo.frame[i] = (linkInfo.frame[f] ^ STUFF_BYTE);
		}else linkInfo.frame[i] = linkInfo.frame[f];
	}
	return i;
}

int dataWrite(int fd, char *packet, int length){
	int n = 0, i;
	unsigned char bcc1, bcc2 = 0x00;

	linkInfo.frame[n++] = FLAG;
	linkInfo.frame[n++] = A_SENDER;
	linkInfo.frame[n++] = linkInfo.sequenceNumber << 6;
	bcc1 = linkInfo.frame[1]^linkInfo.frame[2];
	if (stuff(&linkInfo.frame[n++],bcc1) == 0)
	n++;

	for(i = 0; i < length; i++){
		bcc2 ^=packet[i];
		if (stuff(&linkInfo.frame[n++],packet[i]) == 0)
		n++;
	}
	if (stuff(&linkInfo.frame[n++],bcc2) == 0)
	n++;
	linkInfo.frame[n] = FLAG;

	char frameReceived[COMMAND_LENGTH];
	int commandIsOk;
	resetAlarm();
	write(fd,linkInfo.frame,n);
	do{
		commandIsOk = 1;
		printf("Sending frame...\n");
		write(fd,linkInfo.frame,n);
		n_i_frames++;
		setNextAlarm();
		readFrame(fd, frameReceived, TRANSMITTER);

		if (linkInfo.sequenceNumber == 0)
		commandIsOk = checkCommand(frameReceived, RR_1_FRAME);
		else if (linkInfo.sequenceNumber == 1)
		commandIsOk = checkCommand(frameReceived, RR_0_FRAME);
		if ((checkCommand(frameReceived, REJ_0_FRAME) == 0) || (checkCommand(frameReceived, REJ_1_FRAME) == 0)){
			n_rej_frames++;
			tcflush(fd, TCIOFLUSH);
			write(fd,linkInfo.frame,n);

			commandIsOk = -3;
		}
		if (timeout == TRUE){
			tcflush(fd, TCIOFLUSH);
			write(fd,linkInfo.frame,n);
		}

	}while((commandIsOk != 0) && (numTransmissions < linkInfo.numTransmissions));


	if (numTransmissions >= linkInfo.numTransmissions || commandIsOk  < 0){
		printf("The connection with receiver could not be established.\n");
		return -1;
	}
	linkInfo.sequenceNumber = ((linkInfo.sequenceNumber+1) %2);
	return 0;
}

int dataRead(int length,int fd){
	int i;

	n_i_frames++;


	int size =destuff(length);
	if ((rand() % 20) < 2){
		printf("Generated error\n");
		linkInfo.frame[0] = 0xe7;
	}

	int error = FALSE;
	if(linkInfo.frame[0] == FLAG &&
		linkInfo.frame[size-1] == FLAG &&
		linkInfo.frame[1] == A_SENDER &&
		//linkInfo.frame[2] == linkInfo.sequenceNumber << 6 ||
		linkInfo.frame[3] == (linkInfo.frame[1]^linkInfo.frame[2])){

			unsigned char valbcc2 = 0x00;

			for(i = 4;i < size-2;i++){
				valbcc2 ^= (unsigned char)linkInfo.frame[i];
			}

			if(valbcc2 != (unsigned char)linkInfo.frame[size-2])
			error = TRUE;
		}else error = TRUE;

		if(error == TRUE){
			printf("Rejected frame\n");
			if(linkInfo.frame[2] ==0) write(fd,REJ_1_FRAME,COMMAND_LENGTH);
			else write(fd,REJ_0_FRAME,COMMAND_LENGTH);
			n_rej_frames++;
			return -1;
		}else{

			if(llread(&linkInfo.frame[4],size-6)==0){
				if((linkInfo.frame[2] >> 6) == 0){
					write(fd,RR_1_FRAME,COMMAND_LENGTH);
				}
				else{
					write(fd,RR_0_FRAME,COMMAND_LENGTH);
				}
			}else return -2;
		}
		return 0;
	}

	int closeProtocol(struct applicationLayer app){
		printf("Information Frames: %d\n", n_i_frames);
		printf("REJ Frames: %d\n", n_rej_frames);
		if (app.status == TRANSMITTER){
			printf("Number of timeouts: %d\n", n_timeout);
			return transmitterCloseProtocol(app.fileDescriptor);
		}else if (app.status == RECEIVER){
			//return receiverCloseProtocol(app.fileDescriptor);
		} else {
			return -1;
		}
		return 0;
	}

	int transmitterCloseProtocol(int fd){
		char frameReceived[COMMAND_LENGTH];
		int commandIsOk;
		resetAlarm();
		tcflush(fd, TCIOFLUSH);

		write(fd,DISC_SENDER_FRAME,COMMAND_LENGTH);
		do{
			commandIsOk = 1;
			write(fd,DISC_SENDER_FRAME,COMMAND_LENGTH);
			setNextAlarm();
			readFrame(fd, frameReceived, TRANSMITTER);

			commandIsOk = checkCommand(frameReceived, DISC_RECEIVER_FRAME);

		}while((commandIsOk != 0) && (numTransmissions < linkInfo.numTransmissions));

		if (numTransmissions >= linkInfo.numTransmissions || commandIsOk  < 0){
			printf("The connection with receiver could not be established.\n");
			return -1;
		}
		write(fd,UA_SENDER_FRAME,COMMAND_LENGTH);
		free(linkInfo.frame);
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

		if (timeout == TRUE ){
			printf("Timeout !!!\n");
			n_timeout++;
			return -1;
		}
	}

	return i;
}

int receiverStateManager(int fd) {
	int triedOnce = FALSE;
	int length;

	ReceiverState receiverState = RECEIVING_SET;

	printf("Starting Receiver State Manager...\n");
	while (receiverState != RECEIVING_DONE) {
		switch(receiverState) {
			case RECEIVING_SET:
			printf("Reading SET from transmitter\n");
			length = readFrame(fd, linkInfo.frame, RECEIVER);

			if (checkCommand(linkInfo.frame, SET_FRAME) == 0){
				printf("Writing UA to transmitter\n");
				write(fd,UA_RECEIVER_FRAME,COMMAND_LENGTH);
				triedOnce = TRUE;
			} else if (checkCommand(linkInfo.frame, DISC_SENDER_FRAME) == 0){
				receiverState = RECEIVING_DISC;
			} else if (triedOnce == TRUE && length > 5){
				triedOnce = FALSE;
				receiverState = RECEIVING_DATA;
			}
			break;

			case RECEIVING_DATA:
			if (checkCommand(linkInfo.frame, DISC_SENDER_FRAME) == 0){
				receiverState = RECEIVING_DISC;
			}else{
				dataRead(length,fd);
			}
			length = readFrame(fd, linkInfo.frame, RECEIVER);
			break;

			case RECEIVING_DISC:
			printf("DISC read from transmitter...\n");
			if (checkCommand(linkInfo.frame, DISC_SENDER_FRAME) == 0){
				printf("Writing DISC to transmitter\n");
				write(fd,DISC_RECEIVER_FRAME,COMMAND_LENGTH);
				printf("Waiting for UA\n");
				length = readFrame(fd, linkInfo.frame, RECEIVER);
			}
			if (checkCommand(linkInfo.frame, UA_SENDER_FRAME) == 0){
				receiverState = RECEIVING_UA;
			}
			break;

			case RECEIVING_UA:
			printf("UA read from transmitter\n");
			printf("Closing Receiver Protocol...\n");
			receiverState = RECEIVING_DONE;
			break;

			case RECEIVING_DONE:
			break;

			default:
			break;
		}
	}
	free(linkInfo.frame);
	return 0;
}
