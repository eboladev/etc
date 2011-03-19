import sys
from kdecore import KAboutData, KApplication, KCmdLineArgs, KConfigSkeleton
from kdeui import KConfigDialog, KMainWindow

KCmdLineArgs.init(sys.argv, KAboutData("test", "test", "1.0"))
app = KApplication()
mainWin = KMainWindow(None, "test")
app.setMainWidget(mainWin)
cfg = KConfigSkeleton("test")
dlg = KConfigDialog(mainWin, "settings", cfg)
mainWin.show()
app.exec_loop()
