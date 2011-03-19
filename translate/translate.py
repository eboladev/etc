"""
Translate text using Google Translate
"""

import appuifw
import e32
import socket
import time

def translate(sl, tl, text):
    """
    Translate text from source language (sl) to target language (tl)
    """

    import urllib

    class Opener(urllib.FancyURLopener):
        # Google Translate needs a "real" user agent string
        version = "Opera/5.0 (Linux 2.0.38 i386; U) [en]"


    # Do a POST to Google
    cmd = urllib.urlencode({"sl": sl, "tl": tl})
    data = urllib.urlencode({"hl": "en", "ie": "UTF-8", "oe": "UTF-8",
                             "text": text.encode("utf-8"),
                             "sl": sl, "tl": tl})
    page = Opener().open("http://translate.google.com/translate_t?" + cmd, data)

    # Read and process response
    lines = page.read()
    try:
        start = lines.index("<div id=result_box")
        start = lines.index(">", start)
        end = lines.index("</div>", start)
        return unicode(lines[start + 1:end], "utf-8")
    except:
        return u""
    
        
class TranslateApp(object):
    
    sources = [
        {"l": "ar", "en": u"Arabic"},
        {"l": "bg", "en": u"Bulgarian"},
        {"l": "zh-CN", "en": u"Chinese"},
        {"l": "hr", "en": u"Croatian"},
        {"l": "cs", "en": u"Czech"},
        {"l": "da", "en": u"Danish"},
        {"l": "nl", "en": u"Dutch"},
        {"l": "en", "en": u"English"},
        {"l": "fi", "en": u"Finnish"},
        {"l": "fr", "en": u"French"},
        {"l": "de", "en": u"German"},
        {"l": "el", "en": u"Greek"},
        {"l": "hi", "en": u"Hindi"},
        {"l": "it", "en": u"Italian"},
        {"l": "ja", "en": u"Japanese"},
        {"l": "ko", "en": u"Korean"},
        {"l": "no", "en": u"Norwegian"},
        {"l": "pl", "en": u"Polish"},
        {"l": "pt", "en": u"Portuguese"},
        {"l": "ro", "en": u"Romanian"},
        {"l": "ru", "en": u"Russian"},
        {"l": "es", "en": u"Spanish"},
        {"l": "sv", "en": u"Swedish"}
    ]
    targets = [
        {"l": "ar", "en": u"Arabic"},
        {"l": "bg", "en": u"Bulgarian"},
        {"l": "zh-CN", "en": u"Chinese (Simplified)"},
        {"l": "zh-TW", "en": u"Chinese (Traditional)"},
        {"l": "hr", "en": u"Croatian"},
        {"l": "cs", "en": u"Czech"},
        {"l": "da", "en": u"Danish"},
        {"l": "nl", "en": u"Dutch"},
        {"l": "en", "en": u"English"},
        {"l": "fi", "en": u"Finnish"},
        {"l": "fr", "en": u"French"},
        {"l": "de", "en": u"German"},
        {"l": "el", "en": u"Greek"},
        {"l": "hi", "en": u"Hindi"},
        {"l": "it", "en": u"Italian"},
        {"l": "ja", "en": u"Japanese"},
        {"l": "ko", "en": u"Korean"},
        {"l": "no", "en": u"Norwegian"},
        {"l": "pl", "en": u"Polish"},
        {"l": "pt", "en": u"Portuguese"},
        {"l": "ro", "en": u"Romanian"},
        {"l": "ru", "en": u"Russian"},
        {"l": "es", "en": u"Spanish"},
        {"l": "sv", "en": u"Swedish"},
    ]
    
    def __init__(self):
        appuifw.app.screen = "normal"
        appuifw.app.menu = [(u"Translate", self.onTranslate),
                            (u"Source language", self.onSetSource),
                            (u"Target language", self.onSetTarget),
                            (u"Access point", self.onSetAccessPoint),
                            (u"Clear", self.onClear),
                            (u"Exit", self.onExit)]
        self.lock = e32.Ao_lock()
        self.text = appuifw.Text(u"")
        self.text.font = "normal"
        self.source = "en"
        self.target = "da"
        self.uiLang = "en"
        self.sourceList = None
        self.targetList = None
        self.accessPointId = None
        self.accessPoint = None
        self.lastSensed = time.time()
        self.accelerometer = None
        self.minSensed = 0
        self.maxSensed = 0
        self.sensing = True
        self.sortedLanguageLists = {}
        
        self.restoreSettings()
        self.setTitle()
    
        appuifw.app.body = self.text
        appuifw.app.exit_key_handler = self.onExit

        # Install accelerometer sensor callback
        try:
            import sensor
            accInfo = sensor.sensors()['AccSensor']
            self.accelerometer = \
                sensor.Sensor(accInfo['id'], accInfo['category'])
            if 0: self.accelerometer.connect(self.onAccelerate)
        except:
            pass
        
        self.lock.wait()
    
    
    def setTitle(self):
        appuifw.app.title = \
            u"Translate " + unicode(self.source) + u"|" + unicode(self.target)


    def onTranslate(self):
        self.sensing = False
        try:
            src = self.text.get().split(u'\u2029')[-1]
            if len(src):
                t = translate(self.source, self.target, src)
                self.text.set_pos(self.text.len())
                self.text.add(u"\n" + t + u"\n")
        except:
            pass
        self.sensing = True
        
        
    def onSetAccessPoint(self):
        self.accessPointId = socket.select_access_point()
        self.accessPoint = socket.access_point(self.accessPointId)
        socket.set_default_access_point(self.accessPoint)
        self.saveSettings()
    

    def onExit(self):
        if self.accelerometer:
            if 0: self.accelerometer.disconnect()
        self.lock.signal()
        appuifw.app.set_exit()

        
    def onSetSource(self):
        if not self.sourceList:
            self.sourceList = []
            for l in TranslateApp.sources:
                self.sourceList.append(l[self.uiLang])
        sel = appuifw.selection_list(choices=self.sourceList, search_field=1)
        if sel >= 0:
            self.source = TranslateApp.sources[sel]["l"]
            self.saveSettings()
            self.setTitle()
    

    def onSetTarget(self):
        if not self.targetList:
            self.targetList = []
            for l in TranslateApp.targets:
                self.targetList.append(l[self.uiLang])
        sel = appuifw.selection_list(choices=self.targetList, search_field=1)
        if sel >= 0:
            self.target = TranslateApp.targets[sel]["l"]
            self.saveSettings()
            self.setTitle()
            
            
    def selectLanguage(self, languageList):
        if not self.sortedLanguageLists.has_key(languageList):
            self.sortedLanguageLists[languageList] = []
            for l in languageLists:
                self.sortedLanguageLists[languageList].append(l[self.uiLang])
        return appuifw.selection_list(search_field=1,
            choices=self.sortedLanguageLists[languageList])
            
            
    def onClear(self):
        self.text.delete()
        
        
    def onAccelerate(self, data):
        if not self.sensing:
            return
        now = time.time()
        if data["data_2"] > self.maxSensed:
            self.maxSensed = data["data_2"]
        if data["data_2"] < self.minSensed:
            self.minSensed = data["data_2"]
        if ((self.maxSensed - self.minSensed) > 600) and \
            ((now - self.lastSensed) > 0.9):
            self.onTranslate()
            self.maxSensed = 0
            self.minSensed = 0
    

    def saveSettings(self):
        try:
            f = open("c:\\translate.cfg", "w")
            f.write("%s,%s,%s" % \
                    (self.source, self.target, repr(self.accessPointId)))
            f.close()
        except:
            pass
        
        
    def restoreSettings(self):
        try:
            f = open("c:\\translate.cfg")
            self.source, self.target, accessPointId = f.read().split(",")
            f.close()
            self.accessPointId = int(accessPointId)
        except:
            self.source = "en"
            self.target = "da"
            self.accessPointId = ""
        try:
            self.accessPoint = socket.access_point(self.accessPointId)
            socket.set_default_access_point(self.accessPoint)
        except:
            pass


if __name__ == "__main__":
    TranslateApp()
