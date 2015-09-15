STD=-std=c99 -pedantic
WARN=-Wall -g -Werror
OPT=-O0
INCLUDES=-I./cli/include -I./srv/include -I./protocol/include \
	 -I./atcc/include -I./atcd/include -I./storage/include

BACKEND?=socket
TRANSPORTS=socket fifo shmem queue file
I_BKND=-I./transport/include -I./transport/$(BACKEND)/include

BIN=../bin

FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(CFLAGS) $(INCLUDES) $(I_BKND) -fPIC
LIBS= ncurses m
LINKS=$(foreach lib,$(LIBS),-l$(lib))

TESTSRC=$(wildcard test/*.c)
ATCCSRC=$(wildcard atcc/*.c)
ATCDSRC=$(wildcard atcd/*.c)
CLISRC=$(wildcard cli/*.c)
SRVSRC=$(wildcard srv/*.c) $(wildcard storage/*.c)
COMMSRC=$(wildcard transport/*.c) $(wildcard protocol/*.c)
COMMSRCBKND=$(wildcard transport/$(BACKEND)/*.c)

# The overkill, one size fits all solution to PEOPLE CHANGING HEADERS.
HEADERS=$(shell find . -name *.h)

TESTOBJ=$(TESTSRC:.c=.o)
ATCCOBJ=$(ATCCSRC:.c=.o)
ATCDOBJ=$(ATCDSRC:.c=.o)
CLIOBJ=$(CLISRC:.c=.o)
SRVOBJ=$(SRVSRC:.c=.o)
COMMOBJ=$(COMMSRC:.c=.o)
COMMOBJBKND=$(COMMSRCBKND:.c=.o)
COMM=$(COMMOBJ) $(COMMOBJBKND)

ATC=atc
SRV=${ATC}d
CLI=${ATC}c

SRV_BIN=${BIN}/$(SRV)-$(BACKEND)
CLI_BIN=${BIN}/$(CLI)-$(BACKEND)
ATC_BIN=${BIN}/$(ATC)

# Keep `all` as the first target (here) or I'll kill you.
all:	$(CLI) $(SRV) $(ATC)

%.o: %.c ${HEADERS}
	$(CC) $(FINAL_CFLAGS) -c $< -o $@

$(CLI):	$(CLI_BIN)

$(CLI_BIN):	$(COMM) $(CLIOBJ) $(ATCCOBJ)
	@mkdir -p $(BIN)
	$(CC) $(LDFLAGS) -o $@ $^ $(LINKS)

$(SRV): $(SRV_BIN)

$(SRV_BIN):	$(COMM) $(SRVOBJ) $(ATCDOBJ)
	@mkdir -p $(BIN)
	$(CC) $(LDFLAGS) -o $@ $^ $(LINKS)

$(ATC_BIN):	$(ATCCOBJ) $(ATCDOBJ)
	@mkdir -p $(BIN)
	$(CC) $(LDFLAGS) -o $@ $^ $(LINKS)

$(ATC):	$(ATC_BIN)

clean:
	rm -rf ${BIN}/*
	rm -rf *.a *.log
	find ./ -name *.o -exec rm {} \;

run:	$(CLI) $(SRV) $(ATC)
	valgrind $< > $<.log &

test:	$(TESTOBJ)
	@mkdir -p ${BIN}/test
	$(CC) $(LDFLAGS) -o ${BIN}/$(<:.o=) $<

.PHONY: clean run all $(SRV) $(CLI) test

