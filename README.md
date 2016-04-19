

                            .__       .__
  _________________    ____ |__| _____|  |__    ____  __ __  _____
 /  ___/\____ \__  \  /    \|  |/  ___/  |  \  / ___\|  |  \/     \
 \___ \ |  |_> > __ \|   |  \  |\___ \|   Y  \/ /_/  >  |  /  Y Y  \
/____  >|   __(____  /___|  /__/____  >___|  /\___  /|____/|__|_|  /
     \/ |__|       \/     \/        \/     \//_____/             \/


*     COP5570              |    Parallel and Distibuted Programming
*     Adam Stallard        |    aps10d
*     Program 2            |    Parallel Conways Game of Life
*     Summer C             |    07 / 03 / 15


Summary:

	My project contains these source files:

        gol_omp.c  -  omp parallelism
        gol_thd.c  -  multi-threaded
        gol_prc.c  -  multi-process

		The omp version only adds 3 lines of code to the sequential program.
		It creates a team of threads, and then these threads are assigned
		over two separate for loops.

		The threaded version mimicks the structure of the omp by creating a
		team of threads initallially to be assigned to a task. Each thread
		gets a neighbor count of its partition of the grid, and then waits
		for the other threads to do the same. After each thread is finished,
		they all update the grid, and increment the count variable atomically.


		The multi process uses a series of pipes to connect child to child, and
		parent to child. There are 3 sets of child to child pipes, and 2 sets of
		parent to child. These are used to send ghost arrays, counts, and signals
		for synchronization.



Running:

	> make clean ; clear ; make
	> time ./gol_omp N N
	> time ./gol_thd N N
	> time ./gol_prc N N


	Each will output to the same file so files must be saved if one wishes
	to compare.





