STD=-std=c99 -pedantic
WARN=-Wall -W
OPT=-O2
INCLUDES=-I./cli/include -I./srv/include -I./transport/include

FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(CFLAGS) $(INCLUDES)

CLISRC=$(wildcard cli/*.c)
SRVSRC=$(wildcard srv/*.c)
COMMSRC=$(wildcard transport/*.c)

CLIOBJ=$(CLISRC:.c=.o)
SRVOBJ=$(SRVSRC:.c=.o)
COMMOBJ=$(COMMSRC:.c=.o)

COMM=libcomm.a
SRV=server
CLI=client

LDINC=-L. -lcomm

%.o: %.c
	$(CC) $(FINAL_CFLAGS) -c $< -o $@

$(CLI):	$(COMM) $(CLIOBJ)
	$(CC) $(LDFLAGS) $(LDINC) -o $@ $^

$(SRV): $(COMM) $(SRVOBJ)
	$(CC) $(LDFLAGS) $(LDINC) -o $@ $

$(COMM): $(COMMOBJ)
	$(AR) rcs $@ $(COMMOBJ)

clean:
	rm -rf $(CLIOBJ) $(COMMOBJ) $(SRVOBJ) $(COMM) $(SRV) $(CLI)

all: $(CLI) $(SRV)

.PHONY=clean all

