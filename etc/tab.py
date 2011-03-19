"""
Simple log file viewer.
"""

"""
Copyright (c) 2005 by Akos Polster

Terms and Conditions

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Except as contained in this notice, the name of the copyright holder shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from the
copyright holder.
"""

import os
import sys
import time
from qt import QGridLayout, QIconSet, QLabel, QMultiLineEdit, QPopupMenu, \
    QTabWidget, SIGNAL
from kdecore import KApplication, KCmdLineArgs, KAboutData, i18n, KIcon, \
    KIconLoader, KShortcut
from kdeui import KAction, KActionMenu, KActionSeparator, KFontAction, \
    KFontSizeAction, KMainWindow, KMessageBox, KRadioAction, KStdAction, \
    KTabCtl, KToggleAction, KWindowListMenu
from kfile import KFileDialog
    
"""
File monitor.

Based on code contributed to Python Cookbook by Ed Pascoe (2003).
"""
class Tail:

    def __init__(self, fileName):
        self.fileName = fileName
        self.fd = open(fileName, "r")
        self.started = False
        
    """
    Dump the last 200 or so lines of the file.
    
    @return List of last 200 (approx.) lines in file.
    """
    def start(self):
    
        linesBack = 200
        avgCharsPerLine = 75

        while 1:
            try:
                self.fd.seek(-1 * avgCharsPerLine * linesBack, 2)
            except IOError:
                self.fd.seek(0)
    
            if self.fd.tell() == 0:
                atStart = 1
            else:
                atStart = 0
    
            lines = self.fd.read().split("\n")
            if (len(lines) > (linesBack+1)) or atStart:
                break
    
            avgCharsPerLine = avgCharsPerLine * 1.3
            
        if len(lines) > linesBack:
            start = len(lines) - linesBack - 1
        else:
            start = 0
    
        return lines[start:len(lines) - 1]
        
    """
    Monitor file for changes
    
    @return Text appended to the file since last call.
    """
    def follow(self):
        
        ret = []
        
        if not self.started:
            ret = self.start()
            self.started = True
        
        while 1:
            where = self.fd.tell()
            line = self.fd.readline()
            if not line:
                fdResults = os.fstat(self.fd.fileno())
                try:
                    stResults = os.stat(self.fileName)
                except OSError:
                    stResults = fdResults
                if stResults[1] == fdResults[1]:
                    self.fd.seek(where)
                else:
                    # Inode of the monitored file has changed
                    self.fd = open(self.fileName, "r")
                break
            else:
                ret.append(line)
                
        return ret

"""
File monitor widget.
"""
class Monitor(QMultiLineEdit):
    def __init__(self, tailer):
        QMultiLineEdit.__init__(self)
        self.tailer = tailer
        for line in tailer.follow():
            self.append(line)

"""
Main window.
"""
class MainWin(KMainWindow):

    """
    Constructor.
    """
    def __init__(self, *args):
        apply(KMainWindow.__init__, (self,) + args)
        self.lastDir = "/var/log"
        self.setGeometry(0, 0, 600, 400)
        # self.grid = QGridLayout(self, 1, 1)
        self.tab = QTabWidget(self)
        self.tab.addTab(QMultiLineEdit(self.tab), "Tab")
        self.layout().addWidget(self.tab, 0, 0)
        self.setCaption(str(i18n("%s - lovi")) % str(i18n("(none)")))
        self.initActions()
        self.initMenus()
       
    """
    Initialize actions.
    """
    def initActions(self):
        actions = self.actionCollection()
        self.openAction = KStdAction.open(self.slotOpen, actions)
        self.quitAction = KStdAction.quit(self.slotQuit, actions)
    
    """
    Initialize menus.
    """
    def initMenus(self):
        fileMenu = QPopupMenu(self)
        self.openAction.plug(fileMenu)
        fileMenu.insertSeparator()
        self.quitAction.plug(fileMenu)
        self.menuBar().insertItem(i18n("&File"), fileMenu)
        
        helpMenu = self.helpMenu("")
        self.menuBar().insertItem(i18n("&Help"), helpMenu)
       
    """
    Open a file.
    """
    def slotOpen(self, id = -1):
        fileName = KFileDialog.getOpenFileName(self.lastDir, "*", self, 
            str(i18n("Open Log File")))
        print fileName
        if not fileName.isEmpty():
            self.lastDir = os.path.dirname(str(fileName))
            self.monitor(str(fileName))
       
    """
    Quit application.
    """
    def slotQuit(self, id = -1):
        self.close()

    """
    Monitor a file.
    
    @param  fileName    File to monitor
    """
    def monitor(self, fileName):
        try:
            tailer = Tail(fileName)
        except:
            KMessageBox.error(self, 
                str(i18n("Cannot open file for monitoring:\n%s")) % fileName,
                i18n("Error - lovi"))
            return
        self.tab.addTab(Monitor(tailer), os.path.basename(fileName))

"""
Main program.
"""
description = str(i18n("Simple log file viewer"))
version = "0.1"
about = KAboutData("", "", \
    version, description, KAboutData.License_GPL, \
    "(C) 2005 Akos Polster")
about.addAuthor("Akos Polster", "", "akos@pipacs.com")
KCmdLineArgs.init(sys.argv, about)
# KCmdLineArgs.addCmdLineOptions([("+files", "File to open")])
app = KApplication()
mainWindow = MainWin(None, "lovi#")
mainWindow.show()
app.exec_loop()
