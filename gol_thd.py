import sys, threading
from multiprocessing.pool import ThreadPool as TP

class gol:
    def __init__(self, x, y, n):
        self.x = x
        self.y = y
        self.n_thds = n
        self.cells_per_thd = (x * y) / n
        self.xtra_cells = (x * y) % n
        self.grid = [[0 for _ in range(x)] for _ in range(y)]
        self.newgrid = [[0 for _ in range(x)] for _ in range(y)]
        self.out = sys.stdout
    
    def set_out(self, o):
        self.out = o
    
    def show(self):
        for arr in self.grid:
            for val in arr:
                self.out.write(str(val))
            self.out.write('\n')
    
    def get_grid(self):
        return self.grid

    def set_grid(self, g):
        self.grid = g

    def iter(self):
        for y in range(self.y):
            for x in range(self.x):
                self.update_cell(x, y, self.ncount(x, y))
        self.grid = self.newgrid
    
    def update_cell(self, x, y, n):
        if n <= 1 or n >= 4:
            self.newgrid[y][x] = 0
        elif n == 3:
            self.newgrid[y][x] = 1
        else:
            self.newgrid[y][x] = self.grid[y][x]

    def ncount(self, x, y):
        g = self.grid
        if not (0 <= x < self.x) or not (0 <= y < self.y):
            raise Exception
        if x == 0:
            if y == 0:
                n = g[y][x+1] + g[y+1][x] + g[y+1][x+1]
            elif y == self.y - 1:
                n = g[y][x+1] + g[y-1][x] + g[y-1][x+1]
            else:
                n = g[y-1][x] + g[y+1][x] + g[y-1][x+1] + g[y][x+1] + g[y+1][x+1]
        elif x == self.x - 1:
            if y == 0:
                n = g[y][x-1] + g[y+1][x-1] + g[y+1][x]
            elif y == self.y - 1:
                n = g[y][x-1] + g[y-1][x] + g[y-1][x-1]
            else:
                n = g[y-1][x] + g[y+1][x] + g[y-1][x-1] + g[y][x-1] + g[y+1][x-1]
        else:
            if y == 0:
                n = g[y][x-1] + g[y][x+1] + g[y+1][x-1] + g[y+1][x] + g[y+1][x+1]
            elif y == self.y - 1:
                n = g[y][x-1] + g[y][x+1] + g[y-1][x-1] + g[y-1][x] + g[y-1][x+1]
            else:
                n = g[y-1][x-1] + g[y][x-1] + g[y+1][x-1] + g[y-1][x] + g[y+1][x] + g[y-1][x+1] + g[y][x+1] + g[y+1][x+1]
        return n

    # def calculate(self, beg, cells):
        # x = int(beg % self.x)
        # y = int(beg / self.x)
        # for i in range(cells):
            # c = self.ncount(x, y)
            # if c <= 1 or c >= 4:
                # self.newgrid[y][x] = 0
            # elif c == 3:
                # self.newgrid[y][x] = 1
            # else:
                # self.newgrid[y][x] = self.grid[y][x]
            # x += 1
            # if x == self.x:
                # x = 0
                # y += 1
    
    # def update(self, beg, cells):
        # x = int(beg % self.x)
        # y = int(beg / self.x)
        # for i in range(cells):
            # self.grid[y][x] = self.newgrid[y][x]
            # x += 1
            # if x == self.x:
                # x = 0
                # y += 1


if __name__ == '__main__':
    g = gol(10, 10, 1)
    g.show()
    print '---------------'
    g.grid[0][0] = 1
    g.grid[0][1] = 1
    g.grid[1][1] = 1
    g.grid[1][2] = 1
    g.show()
    for i in range(3):
        print '---------------'
        g.iter()
        g.show()
