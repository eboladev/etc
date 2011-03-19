from PyQt4.QtCore import *
from PyQt4.QtGui import *
import sys

class Applet(QWidget):
    
    def __init__(self):
        QWidget.__init__(self)
        self.menu = QMenu(self)
        self.menu.addAction("Hello", self.hello)
        self.trayIcon = QSystemTrayIcon(self)
        self.trayIcon.setIcon(QIcon("/usr/share/icons/gnome/16x16/actions/up.png"))
        self.trayIcon.setContextMenu(self.menu)
        self.trayIcon.show()

    def hello(self):
        m = QMessageBox(self)
        m.setText("Hello")
        m.exec_();

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setQuitOnLastWindowClosed(False)
    applet = Applet()
    sys.exit(app.exec_())
