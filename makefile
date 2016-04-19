###########################################################
#                                                         #
#  COP5570        |  Parallel and Distibuted Programming  #
#  Adam Stallard  |  aps10d                               #
#  Program 2      |  Parallel Conways Game of Life        #
#  Summer C       |  07 / 03 / 15                         #
#                                                         #
###########################################################

CC = gcc -ansi -std=c99
CCFLAGS = -Wall -pedantic -O3

all : gol_omp gol_thd gol_prc

gol_omp : gol_omp.c
	$(CC) $(CCFLAGS) -fopenmp gol_omp.c -o gol_omp

gol_thd : gol_thd.c
	$(CC) $(CCFLAGS) -lpthread gol_thd.c -o gol_thd

gol_prc : gol_prc.c
	$(CC) $(CCFLAGS) gol_prc.c -o gol_prc

.SUFFIXES :

clean :
	rm gol_omp gol_thd gol_prc
