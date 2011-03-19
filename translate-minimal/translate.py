"""
Translate from English to Danish using Google Translate
"""

import appuifw
import e32
import socket
import time

def translate(text):
    """
    Translate text from English to Danish
    """

    import urllib

    class Opener(urllib.FancyURLopener):
        # Google Translate needs a "real" user agent string
        version = "Opera/5.0 (Linux 2.0.38 i386; U) [en]"

    # Do a POST to Google
    sl = "en"
    tl = "da"
    cmd = urllib.urlencode({"sl": sl, "tl": tl})
    data = urllib.urlencode({"hl": "en", "ie": "UTF-8", "oe": "UTF-8",
                             "text": text.encode("utf-8"),
                             "sl": sl, "tl": tl})
    page = Opener().open("http://translate.google.com/translate_t?" + cmd, data)

    # Read and process response
    lines = page.read()
    start = lines.index("<div id=result_box")
    start = lines.index(">", start)
    end = lines.index("</div>", start)
    return unicode(lines[start + 1:end], "utf-8")
    
        
class TranslateApp(object):
    
    def __init__(self):
        self.lock = e32.Ao_lock()
        self.text = appuifw.Text(u"")
        self.text.font = "normal"
        self.accessPointId = None
        self.accessPoint = None
        
        appuifw.app.screen = "normal"
        appuifw.app.menu = [(u"Translate", self.onTranslate)]
        appuifw.app.title = u"Translate"
        appuifw.app.body = self.text
        appuifw.app.exit_key_handler = self.onExit

        self.lock.wait()
    
    
    def onTranslate(self):
        try:
            src = self.text.get().split(u'\u2029')[-1]
            if len(src):
                t = translate(src)
                self.text.set_pos(self.text.len())
                self.text.add(u"\n" + t + u"\n")
        except:
            pass
        
        
    def onExit(self):
        self.lock.signal()
        appuifw.app.set_exit()

        
if __name__ == "__main__":
    TranslateApp()
