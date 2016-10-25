CC = gcc
CFLAGS = -lm -Wall

all: write read

write: dataLink.h application.h writenoncanonical.c
			$(CC) writenoncanonical.c dataLink.c application.c -o write $(CFLAGS)

read: dataLink.h application.h noncanonical.c
			$(CC) noncanonical.c dataLink.c application.c -o read $(CFLAGS)

clean:
			rm -f write
			rm -f read
