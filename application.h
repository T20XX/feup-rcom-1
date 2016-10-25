#ifndef APPLICATION_H
#define APPLICATION_H

struct applicationLayer {
  int fileDescriptor; /*Descritor correspondente à porta série*/
  int status; /*TRANSMITTER | RECEIVER*/
};

#define T_SIZE 0
#define T_NAME 1
#define T_DATE 2
#define T_PERMISSIONS 3

//volatile int STOP=FALSE;

struct applicationLayer app;

int llopen(const char *port, int status);
int llwrite(const char *file);
int llread();
int llclose();

#endif
