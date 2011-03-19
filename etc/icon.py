#!/usr/bin/env python

False = 0
True  = not False

import sys
from kdecore import KApplication, KAboutData, KIcon, KIconLoader, \
    KCmdLineArgs
from kdeui import KMainWindow, KMessageBox

class MainWin(KMainWindow):
    def __init__ (self, *args):
        apply(KMainWindow.__init__, (self,) + args)
	self.setIcon(KIconLoader().loadIcon("kmail", KIcon.Desktop))

about = KAboutData ("icon", "icon", "1.0", "icon", KAboutData.License_GPL, "")
KCmdLineArgs.init(sys.argv, about)
app = KApplication()
mainWindow = MainWin(None, "main window")
mainWindow.show()
app.exec_loop()
