
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_N 8192

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#define NUM_THRDS 8

char w[MAX_N][MAX_N];
char neww[MAX_N][MAX_N];

int w_X, w_Y;

static int count;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int fin;
struct thd_info {
  pthread_t tid;
  int beg_idx;
  int num_cells;
};

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
    printf("neighborncount: (%d %d) out of bound (0..%d, 0..%d).\n", x,y,
	   w_X, w_Y);
    exit(0);
  }
  if ((y<0) || (y >=w_Y)) {
    printf("neighborncount: (%d %d) out of bound (0..%d, 0..%d).\n", x,y,
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


/********************************************************************
*  Thread function to upate the GOL grid concurrently
********************************************************************/
void *update_table(void *arg) {

	struct thd_info my = *((struct thd_info *)arg);
  int i, x, y, c, loc_count = 0;

  /* get initial (x, y) */
  x = my.beg_idx % w_X;
  y = my.beg_idx / w_X;
  /* count neighbors to generate neww table */
	for (i = 0; i < my.num_cells; ++i) {
		c = neighborcount(x, y);     /* count neighbors */
		if 		(c <= 1)
			neww[y][x] = 0;        /* die of loneliness */
		else if (c >=4)
			neww[y][x] = 0;    /* die of overpopulation */
		else if (c == 3)
			neww[y][x] = 1;            /* becomes alive */
		else
			neww[y][x] = w[y][x];  /* c == 2, no change */
    /* update (x, y) */
    ++x;
    if (x == w_X) {
      x = 0;
      ++y;
    }
	}

  /* wait condition to synchronize threads here */
  pthread_mutex_lock(&mutex);
	++fin;
	pthread_mutex_unlock(&mutex);
  while(1) {
    pthread_mutex_lock(&mutex);
    if (fin == NUM_THRDS) break;
    pthread_mutex_unlock(&mutex);
  }
  pthread_mutex_unlock(&mutex);

  /* get initial (x, y) */
  x = my.beg_idx % w_X;
  y = my.beg_idx / w_X;
  /* copy neww table back over the original */
  for (i = 0; i < my.num_cells; ++i) {
		w[y][x] = neww[y][x];
		loc_count += (w[y][x] == 1);
    /* update (x, y) */
    ++x;
    if (x == w_X) {
      x = 0;
      ++y;
    }
	}

  /* atomic +:reduction on count */
	pthread_mutex_lock(&mutex);
	count += loc_count;
	pthread_mutex_unlock(&mutex);

  /* no return value */
	return NULL;
}

/********************************************************************
*  End thread functions
********************************************************************/



int main(int argc, char *argv[])
{
  int x, y;
  int iter = 0;
  int c;
  int init_count;
  int start_cell;
  int cells_per_thd;
  int extra_cells;
  struct thd_info thd_data[NUM_THRDS];

  /* int count; Made this global for easier thread access */

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

  printf("initial world, population count: %d\n", c);
  if (DEBUG_LEVEL > 10) print_world();


/**************  Initializes variables use by threads  *************/
  cells_per_thd = (w_X * w_Y) / NUM_THRDS;
  extra_cells = (w_X * w_Y) % NUM_THRDS;
/*******************************************************************/


  for (iter = 0; (iter < 200) && (count <50*init_count) &&
	 (count > init_count / 50); iter ++) {

	fin = start_cell = count = 0;

/********************************************************************
*  This is the region that will be made parallel, as
*   it represents one iteration in the GOL.
*  The GOL grid will be divided up so threads can
*   create the new grid concurrently.
********************************************************************/

	/* initialize pthreads to generate a new world */
	for (x=0; x < NUM_THRDS; x++) {
		thd_data[x].beg_idx = start_cell;
    thd_data[x].num_cells = cells_per_thd;
    if (x < extra_cells) {
      ++start_cell;
      ++(thd_data[x].num_cells);
    }
		pthread_create(&(thd_data[x].tid), NULL, &update_table, &thd_data[x]);
    start_cell += cells_per_thd;
	}

	for (x=0; x < NUM_THRDS; x++) pthread_join(thd_data[x].tid, NULL);


/********************************************************************
*  End parallel region
********************************************************************/

    printf("iter = %d, population count = %d\n", iter, count);
    if (DEBUG_LEVEL > 10) print_world();
  }

  {
    FILE *fd;
    if ((fd = fopen("final_world000.txt", "w")) != NULL) {
      for (x=0; x<w_X; x++) {
	for (y=0; y<w_Y; y++) {
          fprintf(fd, "%d", (int)w[y][x]);
	}
	fprintf(fd, "\n");
      }
    } else {
      printf("Can't open file final_world000.txt\n");
      exit(1);
    }
  }


  return 0;
}
