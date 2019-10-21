C = cc
CFLAGS = -std=gnu99 -pedantic -pthread -Wall
OBJECTS = ReverseShell.o Client.o

all: ReverseShell Client

ReverseShell.o: ReverseShell.c
	$(CC) $(CFLAGS) -c ReverseShell.c

Client.o: Client.c
	$(CC) $(CFLAGS) -c Client.c

ReverseShell: ReverseShell.o
	$(CC) ReverseShell.o -o ReverseShell

Client: Client.o
	$(CC) Client.o -o Client

clean:
	rm -f *.o ReverseShell Client