"""
Get available Google Translate languages.
"""

from BeautifulSoup import BeautifulSoup
import urllib
import urllib2

opener = urllib2.build_opener()
opener.addheaders = [('User-agent', "Opera/5.0 (Linux 2.0.38 i386; U) [en]")]

translate_page = opener.open("http://translate.google.com/translate_t")
translate_soup = BeautifulSoup(translate_page)

print "# Source languages"
for language in translate_soup("select", id="old_sl")[0].childGenerator():
    if language['value'] != 'auto':
        print "{\"l\": \"%s\", \"en\": u\"%s\"}," % \
            (language["value"], language.string)

print "\n# Target languages"
for language in translate_soup("select", id="old_tl")[0].childGenerator():
    if language['value'] != 'auto':
        print "{\"l\": \"%s\", \"en\": u\"%s\"}," % \
            (language["value"], language.string)
