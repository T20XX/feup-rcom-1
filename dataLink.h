#ifndef DATALINK_H
#define DATALINK_H

int openProtocol(int status);

int dataWrite(char *packet);

int dataRead(char *packet);

int closeProtocol(int status);

#endif
