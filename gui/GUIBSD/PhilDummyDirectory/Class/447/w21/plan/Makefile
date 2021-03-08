# Cameron Wallace
# Jake Friberg
# Scantron Grading Tool
# Makefile used for scantronGui

CC=gcc
CFLAGS=-g

SRCS=createTest.c
OUTF=$(SRCS:.c=.o)

CPP=g++
CPPFLAGS=-std=c++11 `fltk-config --cxxflags`
LIBS=`fltk-config --ldflags`

CPPSRCS=scantronGui.cxx
CPPOUTF=$(CPPSRCS:.cxx=.o)

scantroncreation: createTest.o scantronGui.o
	$(CPP) $(CPPFLAGS) $(OUTF) $(CPPOUTF) -o scantroncreation $(LIBS)

scantronGui.o: $(CPPSRCS) scantronGui.h
	$(CPP) -c $(CPPFLAGS) $(CPPSRCS)

createTest.o: $(SRCS) createTest.h
	$(CC) -c $(CFLAGS) $(SRCS) -lm

clean:
	rm -f scantroncreation createTest scantronGui $(OUTF) $(CPPOUTF) *.c# *~ *.core
