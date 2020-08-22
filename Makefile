SHELL=/bin/sh
CC=gcc
LIBS=-lpulse

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

listenvol: listenvol.c
	$(CC) $(LIBS) $< -o $@ 

install: listenvol
	install $< /usr/local/bin
