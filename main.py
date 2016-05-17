from gol_thd import gol
from gol_gui import gui

x, y = 30, 40
game = gol(x, y, 10)
interface = gui(x, y, game)

