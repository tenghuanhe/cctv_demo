CC = gcc
CFLAGS = -I. -lcurl -lpillowtalk -pthread

SRC = $(wildcard *.c)

PROGS = $(patsubst %.c, %, $(SRC))

all:
	@echo $(SRC)
	@echo $(PROGS)
all: $(PROGS)
%: %.c
	$(CC) -o $@ $^ $(CFLAGS)
