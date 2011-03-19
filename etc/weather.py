# getTemp.py
#
# Demonstration of the pyparsing module, doing a simple pattern match
# from an HTML page retrieved using urllib
#
# Copyright 2004, by Paul McGuire
#
from pyparsing import Word, Literal, nums
import urllib

city = "Austin, TX"

# define pattern to match within the HTML
# temperature is given in a string of the form:
#        <br><br>67&deg;F<BR>(19&deg;C)
# we want to locate this string within the page, and extract
# the values 67 and 19
makeInt = lambda s,l,t: int(t[0])
integer = Word(nums).setParseAction( makeInt )
currentTempPattern = \
    "<br><br>" + \
    integer.setResultsName("F") + "&deg;F<br>(" + \
    integer.setResultsName("C") + "&deg;C)"

# get current weather for given zip code
noaaURL = "http://www.srh.noaa.gov/zipcity.php?inputstring=%s" % urllib.quote(city)
weatherPage = urllib.urlopen( noaaURL )
weatherReport = weatherPage.read()
weatherPage.close()

# now use scanString to return a generator of matching patterns, and
# invoke next() to get the first (and expected to be only) matching string
print weatherReport
try:
    temps,startloc,endloc = currentTempPattern.scanString( weatherReport ).next()
except StopIteration:
    print "Could not extract temperature data from", noaaURL
else:
    print "Current temp at %s is %d\xb0F (%d\xb0C)" % \
            ( city, temps.F, temps.C )
