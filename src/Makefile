STD=-std=c99 -pedantic
WARN=-Wall -g -Werror
OPT=-O2
INCLUDES=-I./cli/include -I./srv/include -I./transport/include \
	 -I./atcc/include -I./atcd/include
BKND?=socket
I_BKND=-I./transport/$(BKND)/include

BIN=../bin

FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(CFLAGS) $(INCLUDES) $(I_BKND) -fPIC

ATCCSRC=$(wildcard atcc/*.c)
ATCDSRC=$(wildcard atcd/*.c)
CLISRC=$(wildcard cli/*.c)
SRVSRC=$(wildcard srv/*.c)
COMMSRC=$(wildcard transport/*.c)
COMMSRCBKND=$(wildcard transport/$(BKND)/*.c)

ATCCOBJ=$(ATCCSRC:.c=.o)
ATCDOBJ=$(ATCDSRC:.c=.o)
CLIOBJ=$(CLISRC:.c=.o)
SRVOBJ=$(SRVSRC:.c=.o)
COMMOBJ=$(COMMSRC:.c=.o)
COMMOBJBKND=$(COMMSRCBKND:.c=.o)

COMM=$(COMMOBJ) $(COMMOBJBKND)
ATC=atc
SRV=$(ATC)d
CLI=$(ATC)c

SRV_BIN=${BIN}/$(SRV)-$(BKND)
CLI_BIN=${BIN}/$(CLI)-$(BKND)
ATC_BIN=${BIN}/$(ATC)

# Keep `all` as the first target (here) or I'll kill you.
all:	$(CLI) $(SRV) $(ATC)

%.o: %.c
	$(CC) $(FINAL_CFLAGS) -c $< -o $@

$(CLI):	$(CLI_BIN)

$(CLI_BIN):	$(COMM) $(CLIOBJ) $(ATCCOBJ)
	@mkdir -p $(BIN)
	$(CC) $(LDAFLAGS) -o $@ $^

$(SRV): $(SRV_BIN)

$(SRV_BIN):	$(COMM) $(SRVOBJ) $(ATCDOBJ)
	@mkdir -p $(BIN)
	$(CC) $(LDFLAGS) -o $@ $^

$(ATC_BIN):	$(ATCCOBJ) $(ATCDOBJ)
	@mkdir -p $(BIN)
	$(CC) $(LDFLAGS) -o $@ $^

$(ATC):	$(ATC_BIN)

clean:
	rm -rf ${BIN}/*
	rm -rf *.a *.log
	find . -name *.o -exec rm {} \;

run:	$(CLI) $(SRV) $(ATC)
	valgrind $< > $<.log &

.PHONY: clean all

