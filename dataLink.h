#ifndef DATALINK_H
#define DATALINK_H

#include "application.h"


#define FLAG          0x7E
#define A_SENDER      0x03
#define A_RECEIVER    0x01
#define C_SET         0x03
#define C_DISC        0x0B
#define C_UA          0x07
#define C_RR_SUFFIX   0x05
#define C_REJ_SUFFIX  0x01
#define ESCAPE        0x7D
#define STUFF_BYTE    0x20

#define FALSE 0
#define TRUE 1

#define BAUDRATE      B9600
#define MAX_SIZE      256
#define N_TRIES       5
#define TIMEOUT       3

struct linkLayer {
int baudRate;/*Velocidade de transmissão*/
unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
unsigned int timeout;/*Valor do temporizador: 1 s*/
unsigned int numTransmissions; /*Número de tentativas em caso defalha*/
char frame[MAX_SIZE];/*Trama*/
};

struct linkLayer linkInfo;

int openProtocol(struct applicationLayer app);

int dataWrite(char *packet);

int dataRead(char *packet);

int closeProtocol(struct applicationLayer app);

#endif
