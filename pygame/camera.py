#!/usr/bin/python2.4
## -*- coding: utf-8 -*-

# Copyright (c) 2006 INdT - Instituto Nokia de Tecnologia, Elvis Pfutzenreuter.
# Copyright (c) 2007 Nokia Corporation
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import gtk
try:
	import hildon
except ImportError:
	hildon = None

import pygst
pygst.require("0.10")
import gst
import time

def delete_event(widget, event, data=None):
	return False

def destroy(widget, data=None):
	# it is important to stop pipeline so there will be no
	# X-related errors when window is destroyed before the video sink
	pipeline.set_state(gst.STATE_NULL)
	gtk.main_quit()

sink = None
screen = None

def expose_cb(dummy1, dummy2, dummy3):
	print "Expose event"
	sink.set_xwindow_id(screen.window.xid)	

def buffer_cb(pad, buffer):
	# Here you have a chance to do something with the image buffer
	return True

def click_cb(dummy1, dummy2):
	print "Clicked"
	# Canvas has been clicked, here you could trigger for the next
	# buffer_cb() to save the image buffer in a file 
	return True

if hildon:
	window = hildon.Window()
else:
	window = gtk.Window()

window.set_title("Python Mirror")
window.connect("delete_event", delete_event)
window.connect("destroy", destroy)
window.set_border_width(5)

screen = gtk.DrawingArea()
screen.set_size_request(500, 380)
screen.add_events(gtk.gdk.BUTTON_PRESS_MASK)
screen.connect("expose-event", expose_cb, sink);
screen.connect("button_press_event", click_cb);
window.add(screen)

pipeline = gst.Pipeline("mypipeline")

# Alternate sources to run outside of Internet Tablet
# src = gst.element_factory_make("videotestsrc", "src")
# src = gst.element_factory_make("v4lsrc", "src") 
src = gst.element_factory_make("v4l2src", "src")
pipeline.add(src)

flt = gst.element_factory_make("capsfilter", "flt")
pipeline.add(flt)

csp = gst.element_factory_make("ffmpegcolorspace", "csp")
pipeline.add(csp)

caps1 = gst.element_factory_make("capsfilter", "caps1")

# Alternate caps to run outside Internet Tablet (e.g. in a PC with webcam)
# caps1.set_property('caps', gst.caps_from_string("video/x-raw-rgb,bpp=24,depth=24,width=320,height=240,framerate=15/1"))
caps1.set_property('caps', gst.caps_from_string("video/x-raw-rgb,width=640,height=480,bpp=24,depth=24,framerate=15/1"))

pipeline.add(caps1)

csp2 = gst.element_factory_make("ffmpegcolorspace", "csp2")
pipeline.add(csp2)

caps2 = gst.element_factory_make("capsfilter", "caps2")
caps2.set_property('caps', gst.caps_from_string("video/x-raw-yuv"))
pipeline.add(caps2)

sink = gst.element_factory_make("xvimagesink", "sink")
pipeline.add(sink)

pad = src.get_pad('src')
pad.add_buffer_probe(buffer_cb)

src.link(flt)
flt.link(csp)
csp.link(caps1)
caps1.link(csp2)
csp2.link(caps2)
caps2.link(sink)

window.show_all()

pipeline.set_state(gst.STATE_PLAYING)

gtk.main()
