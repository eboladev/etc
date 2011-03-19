import sys
from qt import *
from qtcanvas import *
    
qapp=QApplication(sys.argv)
canvas=QCanvas()
canvasView=QCanvasView(canvas)
item=QCanvasText('hello world',canvas)
item.setX(0)
item.setY(0)
item.show()
canvas.resize(100,100)
canvasView.show()

qapp.connect(qapp, SIGNAL('lastWindowClosed()'), qapp, SLOT('quit()'))
qapp.exec_loop()

