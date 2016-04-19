/*********************************************************************
*                                                                    *
*      Adam Stallard                                                 *
*      COP5570 : Conc, Para, Dist Programming                        *
*      07 / 09 / 15                                                  *
*                                                                    *
*      Multi-process version of Game of Life                         *
*                                                                    *
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_N 8192
#define NUM_PROCS 8

/* pipe indices */
#define SEND_FRWD 0
#define SEND_BKWD 1
#define COUNT_RED 2
#define SEND_SECT 3
#define NOTIFY_CH 4
#define NOTIFY_PT 5

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

/* GOL grids */
char w[MAX_N][MAX_N];
char neww[MAX_N][MAX_N];

/* grid subset from command line */
int w_X, w_Y;

/* useful debugging function */
void test(int proc, int stage)
{
  if (proc == -1)
    fprintf(stderr, "parent is at stage %d\n", stage);
  else
    fprintf(stderr, "child %d is at stage %d\n", proc, stage);
}


void init1(int X, int Y)
{
  int i, j;
  w_X = X,  w_Y = Y;
  for (i=0; i<w_X;i++)
    for (j=0; j<w_Y; j++)
      w[j][i] = 0;

  for (i=0; i<w_X; i++) w[0][i] = 1;
  for (i=0; i<w_Y; i++) w[i][0] = 1;
}

void test_init2()
{
  int i, j;
  w_X = 4;
  w_Y = 6;

  for (i=0; i<w_X;i++)
    for (j=0; j<w_Y; j++)
      w[j][i] = 0;
  w[0][3] = 1;
  w[1][3] = 1;
  w[2][1] = 1;
  w[3][0] = w[3][1] = w[3][2] = w[4][1] = w[5][1] = 1;
}

void print_world()
{
  int i, j;

  for (i=0; i<w_Y; i++) {
    for (j=0; j<w_X; j++) {
      printf("%d", (int)w[i][j]);
    }
    printf("\n");
  }
}

int neighborcount(int x, int y)
{
  int ncount = 0;

  if ((x<0) || (x >=w_X)) {
    printf("neighborcount: (%d %d) out of bound (0..%d, 0..%d).\n", x,y,
	   w_X, w_Y);
    exit(0);
  }
  if ((y<0) || (y >=w_Y)) {
    printf("neighborcount: (%d %d) out of bound (0..%d, 0..%d).\n", x,y,
	   w_X, w_Y);
    exit(0);
  }

  if (x==0) {
    if (y == 0) {
      ncount = w[y][x+1] + w[y+1][x] + w[y+1][x+1];
    } else if (y == w_Y-1) {
      ncount = w[y][x+1] + w[y-1][x] + w[y-1][x+1];
    } else {
      ncount = w[y-1][x] + w[y+1][x] + w[y-1][x+1] + w[y][x+1] + w[y+1][x+1];
    }
  } else if (x == w_X -1) {
    if (y == 0) {
      ncount = w[y][x-1] + w[y+1][x-1] + w[y+1][x];
    } else if (y == w_Y-1) {
      ncount = w[y][x-1] + w[y-1][x] + w[y-1][x-1];
    } else {
      ncount = w[y-1][x] + w[y+1][x] + w[y-1][x-1] + w[y][x-1] + w[y+1][x-1];
    }
  } else { /* x is in the middle */
    if (y == 0) {
      ncount = w[y][x-1] + w[y][x+1] + w[y+1][x-1] + w[y+1][x] + w[y+1][x+1];
    } else if (y == w_Y-1) {
      ncount = w[y][x-1] + w[y][x+1] + w[y-1][x-1] + w[y-1][x] + w[y-1][x+1];
    } else {
      ncount = w[y-1][x-1] + w[y][x-1] + w[y+1][x-1] + w[y-1][x] + w[y+1][x]
              + w[y-1][x+1] + w[y][x+1] + w[y+1][x+1];
    }
  }

  return ncount;
}


/* wrapper for write function */
void my_write(int *fds, void *ref, int nbytes) {
	int bytes;
	if((bytes = write(*fds, ref, nbytes)) < nbytes) {
		fprintf(stderr, "write failed. . . %d bytes written out of %d\n",
			bytes, nbytes);
		exit(-1);
  }
}

/* wrapper for read function */
void my_read(int *fds, void *ref, int nbytes) {
	int bytes;
	if((bytes = read(*fds, ref, nbytes)) < nbytes) {
		fprintf(stderr, "read failed, . . .%d bytes read out of %d\n",
			bytes, nbytes);
		exit(-1);
  }
}

void close_pipes(int pipes[6][NUM_PROCS][2]) {
  int i, j, k;
  for (i = 0; i < 6; ++i)
    for (j = 0; j < NUM_PROCS; ++j)
      for (k = 0; k < 2; ++k)
        close(pipes[i][j][k]);
}


int main(int argc, char *argv[])
{
  int x, y, i;
  int iter = 0;
  int c;
  int count;
  int init_count;
  int reduce_count;

  int p_num;
  pid_t pid;

  int start_row;
  int num_rows;
  int extra_rows;

  int pipes[6][NUM_PROCS][2];
  int par_sig, chd_sig;

  if (argc == 1) {
    printf("Usage: ./a.out w_X w_Y\n");
    exit(0);
  } else if (argc == 2)
    test_init2();
  else /* more than three parameters */
    init1(atoi(argv[1]), atoi(argv[2]));

  c = 0;
  for (x=0; x<w_X; x++) {
    for (y=0; y<w_Y; y++) {
      if (w[y][x] == 1) c++;
    }
  }

  init_count = c;
  count = init_count;

  printf("initial world, population count: %d\n", c);fflush(stdout);
  if (DEBUG_LEVEL > 10) print_world();




  /* determine partition */
  num_rows = w_Y / NUM_PROCS;
  extra_rows = w_Y % NUM_PROCS;
  start_row = 0;




  /* create pipes */
  for (i = 0; i < NUM_PROCS; ++i) {
    if (pipe(pipes[SEND_FRWD][i]) < 0
		|| pipe(pipes[SEND_BKWD][i]) < 0
		|| pipe(pipes[COUNT_RED][i]) < 0
		|| pipe(pipes[SEND_SECT][i]) < 0
    || pipe(pipes[NOTIFY_CH][i]) < 0
    || pipe(pipes[NOTIFY_PT][i]) < 0
		) {
      fprintf(stderr, "Pipe initialization failed . . .\n");
      exit(-1);
    }
  }



  /* create children */
  for (i = 0; i < NUM_PROCS; ++i) {
  	if ((pid = fork()) == -1) {
  	  fprintf(stderr, "fork failed . . .\n");
  	  exit(-1);
  	}
  	else if (pid != 0) { /* parent */
  	  /* keep track of next start_row */
  	  start_row += num_rows;
  	  if (i < extra_rows) ++start_row;
  	}
  	else { /* child */
  	  /* child process number */
  	  p_num = i;
  	  /* decide how many rows it will cover */
  	  if (i < extra_rows) ++num_rows;
  	  /* go to execution stage */
  	  break;
  	}
  }






  /************************** child instructions *****************************/
  if (pid == 0)
  {
    for (iter = 0; iter < 200; ++iter)
    {

      /* check parent status to see if child should keep looping */
      my_read(&pipes[NOTIFY_CH][p_num][0], &par_sig, sizeof(int));
      if (par_sig == 1) break;

      /* send and recieve ghost rows between children */
      if (p_num < NUM_PROCS - 1)
      {
		      my_write(&pipes[SEND_FRWD][p_num][1], &w[start_row + num_rows - 1][0], sizeof(int) * w_X);
      }
	    if (p_num > 0)
      {
		      my_write(&pipes[SEND_BKWD][p_num - 1][1], &w[start_row][0], sizeof(int) * w_X);
      }
      if (p_num < NUM_PROCS - 1)
      {
		      my_read(&pipes[SEND_BKWD][p_num][0], &w[start_row + num_rows][0], sizeof(int) * w_X);
      }
	    if (p_num > 0)
      {
		      my_read(&pipes[SEND_FRWD][p_num - 1][0], &w[start_row - 1][0], sizeof(int) * w_X);
      }


      /* get counts and generate neww grid */
      for (x = 0; x < w_X; ++x)
      {
    		for (y = start_row; y < start_row + num_rows; ++y)
        {
    			c = neighborcount(x, y);     /* count neighbors */
    			if 		(c <= 1)
    				neww[y][x] = 0;        /* die of loneliness */
    			else if (c >=4)
    				neww[y][x] = 0;    /* die of overpopulation */
    			else if (c == 3)
    				neww[y][x] = 1;            /* becomes alive */
    			else
    				neww[y][x] = w[y][x];  /* c == 2, no change */
        }
      }

      /* copy back over old grid w and get a count for partition */
      count = 0;
      for (x = 0; x < w_X; ++x)
      {
    		for (y = start_row; y < start_row + num_rows; ++y)
        {
            w[y][x] = neww[y][x];
            count += (w[y][x] == 1);
        }
      }


      /* we now have a local count and must reduce to the parent proc */
      if (p_num < NUM_PROCS - 1)
      {
		      my_read(&pipes[COUNT_RED][p_num + 1][0], &reduce_count, sizeof(int));
          count += reduce_count;
      }
		  my_write(&pipes[COUNT_RED][p_num][1], &count, sizeof(int));


      /* update world each iter if debug set */
      if (DEBUG_LEVEL > 10)
      {
        /* write partitioned GOL blocks to parent procress */
        for (y = start_row; y < start_row + num_rows; ++y)
        {
            my_write(&pipes[SEND_SECT][p_num][1], &w[y][0], sizeof(int) * w_X);
            my_read(&pipes[NOTIFY_PT][p_num][0], &chd_sig, sizeof(int));
        }
      }

    } /* END OF FOR LOOP */

    /* write partitioned GOL blocks to parent procress */
    for (y = start_row; y < start_row + num_rows; ++y)
    {
        my_write(&pipes[SEND_SECT][p_num][1], &w[y][0], sizeof(int) * w_X);
        my_read(&pipes[NOTIFY_PT][p_num][0], &chd_sig, sizeof(int));
    }

    /* successful child exit */
    exit(0);


  } /********************* END OF CHILD INSTRUCTIONS *************************/






  else /*********************** parent instructions **************************/
  {
    par_sig = chd_sig = 0;
      for (iter = 0; (iter < 200) && (count < 50 * init_count) &&
  	   (count > init_count / 50); iter ++)
     {

      /* send signals to keep child loops going */
      for (i = 0; i < NUM_PROCS; ++i)
        my_write(&pipes[NOTIFY_CH][i][1], &par_sig, sizeof(int));

      /* recieve final count from head child */
      count = 0;
      my_read(&pipes[COUNT_RED][0][0], &count, sizeof(int));

      printf("iter = %d, population count = %d\n", iter, count);
      if (DEBUG_LEVEL > 10) {

        /* recieve updated partitions from each child */
        start_row = 0;
        for (i = 0; i < NUM_PROCS; ++i)
        {
          for (y = start_row; y < num_rows; ++y)
          {
            my_read(&pipes[SEND_SECT][i][0], &w[y][0], sizeof(int) * w_X);
            my_write(&pipes[NOTIFY_PT][i][1], &chd_sig, sizeof(int));
          }
          start_row += num_rows;
          if (i < extra_rows) {
            my_read(&pipes[SEND_SECT][i][0], &w[start_row][0], sizeof(int) * w_X);
            my_write(&pipes[NOTIFY_PT][i][1], &chd_sig, sizeof(int));
            ++start_row;
          }
        }

        print_world();
        for (i = 0; i < NUM_PROCS; ++i)
          my_write(&pipes[NOTIFY_CH][i][1], &par_sig, sizeof(int));

      }

    } /* END OF FOR LOOP */


    /* send signals to make children exit loop */
    par_sig = 1;
    for (i = 0; i < NUM_PROCS; ++i)
      my_write(&pipes[NOTIFY_CH][i][1], &par_sig, sizeof(int));


    /* recieve updated partitions from each child */
    start_row = 0;
    for (i = 0; i < NUM_PROCS; ++i)
    {
      for (y = start_row; y < start_row + num_rows; ++y)
      {
        my_read(&pipes[SEND_SECT][i][0], &w[y][0], sizeof(int) * w_X);
        my_write(&pipes[NOTIFY_PT][i][1], &chd_sig, sizeof(int));
      }
      start_row += num_rows;
      if (i < extra_rows) {
        my_read(&pipes[SEND_SECT][i][0], &w[y][0], sizeof(int) * w_X);
        my_write(&pipes[NOTIFY_PT][i][1], &chd_sig, sizeof(int));
        ++start_row;
      }
    }


  } /********************* END OF PARENT INSTRUCTIONS ************************/




  /* only master process will execute the following code */
  for (i = 0; i < NUM_PROCS; ++i)
    wait(NULL);

  /* close pipes */
  close_pipes(pipes);

  {
    FILE *fd;
    if ((fd = fopen("final_world000.txt", "w")) != NULL)
    {
      for (x=0; x<w_X; x++)
      {
	      for (y=0; y<w_Y; y++)
           fprintf(fd, "%d", (int)w[y][x]);
	      fprintf(fd, "\n");
      }
    }
    else
    {
      printf("Can't open file final_world000.txt\n");
      exit(1);
    }
  }

  /* YAYY!!! :D */
  return 0;
}
