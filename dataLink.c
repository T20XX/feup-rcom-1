#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "dataLink.h"
#include "utils.h"

int transmitterOpenProtocol();
int receiverOpenProtocol();

int openProtocol(int status){
  if (status == TRANSMITTER){
    return transmitterOpenProtocol();
  }else if (status == RECEIVER){
    return receiverOpenProtocol();
  } else {
    return -1;
  }
  return 0;
}

int transmitterOpenProtocol(){

  return 0;
}

int receiverOpenProtocol(){

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
