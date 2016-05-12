import Tkinter as tk


class gui:
    def __init__(self, master):
        frame = tk.Frame(master)
        frame.pack()
        self.button = tk.Button(
            frame, text='Quit', fg='blue', command=frame.quit)
        self.button.pack(side=tk.LEFT)
        self.title = tk.Button(
            frame, text='Solr Population for SAMOS data in DOMS', 
            command=self.say_hi)
        self.title.pack(side=tk.LEFT)
        self.canvas = tk.Canvas(master, width=200, height=100)
        self.canvas.pack()
        self.lineA = self.canvas.create_line(0, 0, 200, 100)
        self.lineB = self.canvas.create_line(0, 100, 200, 0, fill='red', dash=(4, 4))
        self.rectA = self.canvas.create_rectangle(50, 25, 150, 75, fill='blue')


    def say_hi(self):
        print 'hi there, everyone!'
        self.canvas.delete(self.lineA)
        self.canvas.delete(self.lineB)
        self.canvas.delete(self.rectA)

def main():
    root = tk.Tk()
    g = gui(root)
    root.mainloop()
    root.destroy()




if __name__ == '__main__':
    main()
