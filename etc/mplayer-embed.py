#!/usr/bin/env python
# See http://www.mplayerhq.hu/DOCS/tech/slave.txt
# and mplayer -input cmdlist
from qt import *
import os, atexit


class MPlayerWidget(QWidget):

    CMD = "mplayer -slave -quiet -noconsolecontrols -nomouseinput -vo %(VO)s -ao %(AO)s -wid %(WID)s %(FILENAME)r"

    
    CFG = dict(
        AO = "alsa",
        VO = "xv" #VO = "x11"
    )

    
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.process = None
        self.layout = QVBoxLayout(self)
        
        self.view = QLabel(self)
        self.layout.addWidget(self.view)

        self.controls = QHBoxLayout(self)
        self.layout.addLayout(self.controls)

        self.pauseButton = QToolButton(self)
        self.pauseButton.setMaximumHeight(32)
        self.pauseButton.setText("&Pause")
        self.pauseButton.setAutoRaise(True)
        self.pauseButton.setToggleButton(True)
        self.connect(self.pauseButton, SIGNAL("clicked()"), self.toggle_pause)
        self.controls.addWidget(self.pauseButton)

        self.muteButton = QToolButton(self)
        self.muteButton.setText("&Mute")
        self.muteButton.setAutoRaise(True)
        self.muteButton.setToggleButton(True)
        self.connect(self.muteButton, SIGNAL("clicked()"), self.toggle_mute)
        self.controls.addWidget(self.muteButton)
        
        self.quitButton = QToolButton(self)
        self.quitButton.setText("&Close")
        self.quitButton.setAutoRaise(True)
        self.connect(self.quitButton, SIGNAL("clicked()"), self.exit)
        self.controls.addWidget(self.quitButton)
        

    def start(self, filename):
        self.pause_flag = False
        self.fullscreen_flag = False
        self.show()
        self.view.setText("Loading %s..." % filename)
        qApp.processEvents()
        
        self.CFG["WID"] = self.view.winId()
        self.CFG["FILENAME"] = filename
        self.process = os.popen(self.CMD % self.CFG, "w", 1)

        atexit.register(self.exit)



    def play(self, filename):
        if self.process:
            self("quit")
            self.process.close()

        self.start(filename)
        

    def osd(self, msg):
        self("osd_show_text %s" % msg)
        

    def exit(self):
        if self.process:
            self("pause")
            self("quit 0")
            self.process.close()
            self.process = None
        self.close()


    def __call__(self, cmd):
        if self.process:
            print "*", cmd
            self.process.write("\n%s\n" % cmd)


    def __del__(self):
        self.exit()
        

    def load(self, url):
        self.CFG["FILENAME"] = url
        self("loadfile %s" % url)


    def toggle_mute(self):
        self("mute")


    def toggle_pause(self):
        if not self.pause_flag:
            self.osd("Paused")
        self("pause")
        self.pause_flag = not self.pause_flag

    
    def pause(self):
        if not self.pause_flag:
            self.toggle_pause()


    def unpause(self):
        if self.pause_flag:
            self.toggle_pause()


    def fullscreen(self):
        if not self.fullscreen_flag:
            self("vo_fullscreen")
            self.fullscreen_flag = True


    def windowed(self):
        if self.fullscreen_flag:
            self("vo_fullscreen")
            self.fullscreen_flag = False



if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print "Usage: %s FILENAME" % sys.argv[0]
        sys.exit()
    app = QApplication(sys.argv)
    win = MPlayerWidget()
    win.resize(320, 240)
    app.setMainWidget(win)
    win.play(sys.argv[1])
    app.exec_loop()
