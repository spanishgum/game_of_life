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
    
    def iter(self):
        tpool = TP(self.n_thds)
        beg, end, count = 0, 0, 0
        thd_dat = []
        for i in range(self.n_thds):
            thd_dat.append({ 'beg' : beg, 'cells' : self.cells_per_thd, 'id' : None })
            if i < self.xtra_cells:
                beg += 1
                thd_dat[-1]['cells'] += 1
        tpool.map(self.calculate, thd_dat)
        tpool.close()
        tpool.join()
        tpool.map(self.update, thd_dat)
        tpool.close()
        tpool.join()
        
    # def iter(self):
        # beg, end, count = 0, 0, 0
        # thd_dat = [None] * self.n_thds
        # for i in range(self.n_thds):
            # thd_dat[i] = { 'beg' : beg, 'cells' : self.cells_per_thd, 'id' : None }
            # if i < self.xtra_cells:
                # beg += 1
                # thd_dat[i]['cells'] += 1
            # thd_dat[i]['id'] = threading.Thread(target = self.calculate, args = (thd_dat[i],))
            # thd_dat[i]['id'].start()
        # for i in range(self.n_thds):
            # thd_dat[i]['id'].join()
        # for i in range(self.n_thds):
            # thd_dat[i]['id'] = threading.Thread(target = self.update, args = (thd_dat[i],))
            # thd_dat[i]['id'].start()
        # for i in range(self.n_thds):
            # thd_dat[i]['id'].join()

    def ncount(x, y):
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

    def calculate(beg, cells):
        x = int(beg % self.x)
        y = int(beg / self.x)
        for i in range(cells):
            c = self.ncount(x, y)
            if c <= 1 or c >= 4:
                self.newgrid[y][x] = 0
            elif c == 3:
                self.newgrid[y][x] = 1
            else:
                self.newgrid[y][x] = self.grid[y][x]
            x += 1
            if x == self.x:
                x = 0
                y += 1
    
    def update(beg, cells):
        x = int(beg % self.x)
        y = int(beg / self.x)
        for i in range(cells):
            self.grid[y][x] = self.newgrid[y][x]
            x += 1
            if x == self.x:
                x = 0
                y += 1



def main():
    g = gol(10, 10, 1)
    g.show()
    print '---------------'
    g.iter()
    g.show()

if __name__ == '__main__':
    main()
