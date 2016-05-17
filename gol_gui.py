import Tkinter as tk, time


class gui:
    def __init__(self, rows, cols, game):
        self.master = tk.Tk()
        self.game = game
        self.rows = rows
        self.cols = cols
        self.colors = ('black', 'white')
        self.grid = [[0 for _ in range(self.rows)] for _ in range(self.cols)]
        self.rects = [[None for _ in range(self.rows)] for _ in range(self.cols)]

        frame = tk.Frame(self.master)
        frame.pack()

        self.quit_button = tk.Button(frame, text='Quit', fg='blue', command=frame.quit)
        self.quit_button.pack(side=tk.RIGHT)

        self.title_button = tk.Button(frame, text='Game of Life', command=self.beg)
        self.title_button.pack(side=tk.LEFT)

        self.canvas = tk.Canvas(self.master, width=20*self.cols, height=20*self.rows, borderwidth=5, background='white')
        self.canvas.pack()
        self.canvas.bind('<Button-1>', self.click_handler)
        self.master.update()

        self.col_w = self.canvas.winfo_width() / self.cols
        self.row_h = self.canvas.winfo_height() / self.rows
        self.init_grid()

        self.master.mainloop()


    def beg(self):
        self.game.set_grid(self.grid)
        for _ in range(20):
            self.game.iter()
            self.update(self.game.get_grid())
            self.master.update()
            time.sleep(1)

    def end(self):
        self.master.destroy()

    def init_grid(self):
        for c in range(self.cols):
            for r in range(self.rows):
                self.rects[c][r] = self.canvas.create_rectangle(c * self.col_w, r * self.row_h, (c+1) * self.col_w, (r+1) * self.row_h, fill = self.colors[self.grid[c][r]])

    def update_cell(self, col, row):
        self.canvas.itemconfig(self.rects[col][row], fill = self.colors[self.grid[col][row]])

    def update(self, grid):
        self.grid = grid
        for c in range(self.cols):
            for r in range(self.rows):
                self.update_cell(c, r)

    def click_handler(self, event):
        col = event.x / self.col_w
        row = event.y / self.row_h
        print col, row
        self.grid[col][row] = 1 - self.grid[col][row]
        self.canvas.itemconfig(self.rects[col][row], fill = self.colors[self.grid[col][row]])

if __name__ == '__main__':
    gui(10, 10, None).beg()
