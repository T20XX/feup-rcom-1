CC = gcc
CFLAGS = -lm -Wall

all: clean write read

write: dataLink.h application.h utils.h writenoncanonical.c
			$(CC) writenoncanonical.c dataLink.c application.c -o write $(CFLAGS)

read: dataLink.h application.h utils.h noncanonical.c
			$(CC) noncanonical.c dataLink.c application.c -o read $(CFLAGS)

clean:
			rm -f write
			rm -f read
