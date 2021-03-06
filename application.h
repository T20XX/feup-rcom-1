#ifndef APPLICATION_H
#define APPLICATION_H


struct applicationLayer {
  int fileDescriptor; /*Descritor correspondente à porta série*/
  int status; /*TRANSMITTER | RECEIVER*/
};

#define TRANSMITTER 0
#define RECEIVER 1

#define T_SIZE 0
#define T_NAME 1
#define T_DATE 2
#define T_PERMISSIONS 3


struct applicationLayer app;

int llopen(const char *port, int status, int baudrate, int framesize, int noftries, int noftimeout);
int llwrite(const char *file);
int llread(char *packet, int length);
int llclose();

#endif
