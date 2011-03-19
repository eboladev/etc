#!/usr/bin/env python

False = 0
True  = not False

import sys
import qt
from kdecore import KApplication, KAboutData, KIcon, KIconLoader, \
    KCmdLineArgs
from kdeui import KMainWindow, KMessageBox

class MainWin(KMainWindow):
    def __init__ (self, *args):
        apply(KMainWindow.__init__, (self,) + args)
	self.setIcon(KIconLoader().loadIcon("kmail", KIcon.Desktop))
	self.b = qt.QPushButton(self)
        self.b.setFixedSize(10, 10)
        p = qt.QPixmap("one.png")
        self.b.setPixmap(p)
        # self.b.setText("hello")
        self.b.show()
        self.setCentralWidget(self.b)
        self.show()

about = KAboutData ("icon", "icon", "1.0", "icon", KAboutData.License_GPL, "")
KCmdLineArgs.init(sys.argv, about)
app = KApplication()
mainWindow = MainWin(None, "main window")
mainWindow.show()
app.exec_loop()
