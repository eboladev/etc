import sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class QuarkList(QListWidget):
    """List widget with custom colors"""
    def __init__(self, parent = None):
        QListWidget.__init__(self, parent)
        palette = QPalette(self.palette())
        palette.setColor(QPalette.Background, Qt.black);
        palette.setColor(QPalette.Base, Qt.black)
        palette.setColor(QPalette.Text, QColor(255, 180, 0))
        palette.setColor(QPalette.Highlight, QColor(255, 180, 0))
        palette.setColor(QPalette.HighlightedText, Qt.black)
        self.setPalette(palette)
        self.setFixedSize(200, 200)


class PhoneBook(QuarkList):
    """Phone book mock-up"""
    entries = [
        ["Farzad Keynejad", "+353 872917107"],
        ["Finn Kornbo", "+45 46914425"],
        ["Heike Fix", "+49 1726970434"],
        # ["Kinga Szekeres", "+49 1601411487"]
    ]
    def __init__(self, parent = None):
        QuarkList.__init__(self, parent)
        for name, phone in PhoneBook.entries:
            QListWidgetItem(name + "\n  " + phone, self)


class Idle(QLabel):
    """Idle display mock-up"""
    def __init__(self, parent = None):
        QLabel.__init__(self, parent)
        self.setPixmap(QPixmap.fromImage(QImage("idle.png")))


class Messaging(QuarkList):
    """SMS viewer mock-up"""
    def __init__(self, parent = None):
        QuarkList.__init__(self, parent)
        QListWidgetItem("Heike Fix\nLunch at 12:30!", self)


class Quark(QGraphicsView):

    """Main window"""

    def __init__(self, parent = None):
        QGraphicsView.__init__(self, parent)
        self.setWindowTitle("Quark 1")
        self.setFixedSize(204, 204)

        # Create "application" widgets
        self.phoneBook = PhoneBook()
        self.idle = Idle()
        self.messaging = Messaging()

        # Home screen indicators
        self.indicators = [Indicator(0), Indicator(1), Indicator(2)]
        self.indicatorItem = None
        self.indicatorTimer = QTimer()
        self.indicatorTimer.setSingleShot(True)
        self.indicatorTimer.setInterval(1000)
        self.connect(self.indicatorTimer, SIGNAL("timeout()"),
                     self.hideIndicator)

        # Create graphics scene
        self.scene = QGraphicsScene()
        self.scene.setBackgroundBrush(QBrush(Qt.black))

        self.phoneBookItem = self.scene.addWidget(self.phoneBook)
        self.messagingItem = self.scene.addWidget(self.messaging)
        self.idleItem = self.scene.addWidget(self.idle)
        self.items = [self.idleItem, self.phoneBookItem, self.messagingItem]
        self.currentItem = len(self.items) - 1

        # Set up view for the scene
        self.setScene(self.scene)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setSceneRect(0, 0, 200, 200)

        # Time line for transition animations
        self.timeLine = QTimeLine()
        self.timeLine.setCurveShape(QTimeLine.EaseInOutCurve)
        self.timeLine.setLoopCount(1)
        self.timeLine.setDuration(300)
        self.connect(self.timeLine, SIGNAL("finished()"), self.showIndicator)

        # Empty graphics item for inactive animations
        self.noItem = QGraphicsLineItem()
        self.noItem.setVisible(False)

        # Animation for item coming from left
        self.inFromLeftAnim = QGraphicsItemAnimation()
        self.inFromLeftAnim.setTimeLine(self.timeLine)
        for i in xrange(250):
            self.inFromLeftAnim.setPosAt(i / 250.0, QPointF(-249 + i, 0))

        # Animation for item going out to left
        self.outToLeftAnim = QGraphicsItemAnimation()
        self.outToLeftAnim.setTimeLine(self.timeLine)
        for i in xrange(250):
            self.outToLeftAnim.setPosAt(i / 250.0, QPointF(-1 - i, 0))

        # Animation for item coming from right
        self.inFromRightAnim = QGraphicsItemAnimation()
        self.inFromRightAnim.setTimeLine(self.timeLine)
        for i in xrange(250):
            self.inFromRightAnim.setPosAt(i / 250.0, QPointF(249 - i, 0))

        # Animation for item going out to right
        self.outToRightAnim = QGraphicsItemAnimation()
        self.outToRightAnim.setTimeLine(self.timeLine)
        for i in xrange(250):
            self.outToRightAnim.setPosAt(i / 250.0, QPointF(1 + i, 0))

        self.showNextApp()
        self.update()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Right:
            self.showNextApp()
        elif event.key() == Qt.Key_Left:
            self.showPrevApp()
        else:
            return QGraphicsView.keyPressEvent(self, event)

    def showNextApp(self):
        """Show next application by shifting the view to the right"""
        outItem = self.currentItem
        self.currentItem += 1
        if self.currentItem == len(self.items):
            self.currentItem = 0

        self.inFromRightAnim.setItem(self.items[self.currentItem])
        self.outToLeftAnim.setItem(self.items[outItem])
        self.inFromLeftAnim.setItem(self.noItem)
        self.outToRightAnim.setItem(self.noItem)

        self.scene.setFocusItem(self.items[self.currentItem])
        self.timeLine.start()

    def showPrevApp(self):
        """Show previous application by shifting the view to the left"""
        outItem = self.currentItem
        self.currentItem -= 1
        if self.currentItem < 0:
            self.currentItem = len(self.items) - 1

        self.inFromLeftAnim.setItem(self.items[self.currentItem])
        self.outToRightAnim.setItem(self.items[outItem])
        self.inFromRightAnim.setItem(self.noItem)
        self.outToLeftAnim.setItem(self.noItem)

        self.scene.setFocusItem(self.items[self.currentItem])
        self.timeLine.start()

    def showIndicator(self):
        """Show home screen indicator"""
        print "Quark.showIndicator %d" % self.currentItem
        if self.indicatorItem:
            self.scene.removeItem(self.indicatorItem)
        self.indicatorItem = \
            self.scene.addPixmap(self.indicators[self.currentItem])
        self.indicatorItem.setZValue(1)
        self.indicatorTimer.stop()
        self.indicatorTimer.start()

    def hideIndicator(self):
        """Hide home screen indicator"""
        print "Quark.hideIndicator %d" % self.currentItem
        self.scene.removeItem(self.indicatorItem)
        self.indicatorItem = None

class Indicator(QPixmap):
    """Hoem screen indicator"""
    def __init__(self, index):
        QPixmap.__init__(self, "hs%d.png" % (index + 1))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = QDialog()
    window.setWindowTitle("Quark 1")
    quark = Quark(window)
    quit = QPushButton("Quit")
    window.connect(quit, SIGNAL("clicked()"), sys.exit)
    layout = QVBoxLayout()
    layout.addWidget(quark);
    layout.addWidget(quit);
    window.setLayout(layout);
    app.setActiveWindow(window)
    window.show()
    sys.exit(app.exec_())
