###########################################################
#                                                         #
#  COP5570        |  Parallel and Distibuted Programming  #
#  Adam Stallard  |  aps10d                               #
#  Program 2      |  Parallel Conways Game of Life        #
#  Summer C       |  07 / 03 / 15                         #
#                                                         #
###########################################################

CC = gcc -ansi -std=c99
CCFLAGS = -Wall -pedantic -O3 -fopenmp
PRGS := $(patsubst %.c,%,$(wildcard *.c))

all : $(PRGS)

% : %.c
	$(CC) $(CCFLAGS) $@.c -o $@

.SUFFIXES :

clean :
	rm $(PRGS)
