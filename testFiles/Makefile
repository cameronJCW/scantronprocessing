# Cameron Wallace
# October 27, 2020
# Scantron Grading Tool Project
# Makefile used for createTest

CC=gcc
CFLAGS=-g -Wall

SRCS = createTest.c
OUTF = ${SRCS:.c=.o}

createTest: $(OUTF)
	$(CC) $(CFlags) -o createTest $(OUTF) -lm

clean:
	rm -f createTest $(OUTF) *.c# core
