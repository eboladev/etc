'''
Sensor Analyzer for raw accelometer data

Copyright (c) 2008 Jouni Miettunen
http://jouni.miettunen.googlepages.com/

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

1.2  2008-05-12 Added Zooming, Graph and Vectors, minor fixes
1.1  2008-05-08 Visualize raw sensor data
     Bars idea based on Aapo Rista modified verlet.py
1.0  2008-05-07 Based on Sensor Sample + debug code to see raw sensor data

'''
import e32
import appuifw
import graphics
import key_codes

VERSION = u'1.2'

#DEBUG = True
DEBUG = False

# Display modes
VIEW_MIN = 10
VIEW_BAR = VIEW_MIN
VIEW_VECTOR = VIEW_MIN + 1
VIEW_GRAPH = VIEW_MIN + 2
VIEW_MAX = VIEW_MIN + 2
my_view = VIEW_MIN

if DEBUG:
    import sys
    import os
    import time
    import linecache

    # Create own files in current installation directory
    try:
        raise Exception
    except Exception:
        frame = sys.exc_info()[2].tb_frame
        filepath = frame.f_code.co_filename

    # Setup special debug files
    filedir, filename = os.path.split(filepath)
    (a, b) = filename.split('.')
    FILE_PIC = os.path.join(filedir, a + ".png")
    FILE_LOG = os.path.join(filedir, a + ".debug.txt")
    my_log = file(FILE_LOG, "w")
    sys.stdout = my_log     # Forward output to log file
    sys.stderr = my_log     # Forward errors to log file

    # Print debug header
    print filepath
    print "Started", time.ctime()
    print "-"*60

def cb_debug(frame, event, arg):
    ''' My own trace/profile function '''
    if not frame:
        return cb_debug
    filename = frame.f_code.co_filename
    name = frame.f_code.co_name
    lineno = frame.f_lineno
    line = linecache.getline(filename, lineno)
    out_str = "%s\t%s@%s: %s" % (filename, name, lineno, line.rstrip())

    if event == "line":
        pass
    elif event == "call":
        out_str = "===== CALL " + out_str
    elif event == "return":
        out_str = "===== RETURN " + out_str
    # TODO: exception, c-call, c-return, c-exception
    print out_str
    return cb_debug

if DEBUG:
    # Trace hook is called for EACH LINE that is executed
    # Use for more thorought but slower debugging
    sys.settrace(cb_debug)
    #
    # Profile hook is only called for EACH FUNCTION
    # Use for less thorough but faster debugging
    #sys.setprofile(cb_debug)
# DEBUG

# SENSOR: if exception, it's not available at all
SENSOR_ACC = SENSOR_TAP = False

try:
    import sensor
    from sensor import orientation
    SENSOR = True                # Sensor interface available
    if DEBUG: print "SENSOR OK"
except ImportError:
    SENSOR = False               # Sensor interface not available
    if DEBUG: print "SENSOR NA"

# Global definitions for raw sensor data
use_data_1 = True
use_data_2 = True
use_data_3 = True
acc_data_1 = 0
acc_data_2 = 0
acc_data_3 = 0
min_data = 9999
min_data_1 = 9999
min_data_2 = 9999
min_data_3 = 9999
max_data = 9999
max_data_1 = -9999
max_data_2 = -9999
max_data_3 = -9999

# Colors for different purposes
RGB_BLACK = (0, 0, 0)
RGB_WHITE = (255, 255, 255)
RGB_RED = (255, 0, 0)
RGB_GREEN = (0, 255, 0)
RGB_BLUE = (0, 0, 255)
RGB_YELLOW = (255, 255, 0)

# Global variables
screen_x = 240
screen_y = 320
origo_x = 120
origo_y = 160
canvas = None
img = None
im1 = None
# MAGIC
my_zoom = 3
my_scale = my_zoom * 120

def reset_data():
    ''' Restart collecting raw sensor data '''
    if DEBUG: print "reset_data: ", time.ctime(), locals()
    global min_data, min_data_1, min_data_2, min_data_3
    global max_data, max_data_1, max_data_2, max_data_3
    acc_data_1 = 0
    acc_data_2 = 0
    acc_data_3 = 0
    min_data = 9999
    min_data_1 = 9999
    min_data_2 = 9999
    min_data_3 = 9999
    max_data = -9999
    max_data_1 = -9999
    max_data_2 = -9999
    max_data_3 = -9999

def handle_sensor_raw(a_data):
    ''' My own handler for raw sensor events '''
    #if DEBUG: print "handle_sensor_raw: ", time.ctime(), locals()
    if SENSOR_ACC == False:
        return

    # Non-filtered raw data
    #handle_orientation:  {'a_data':
    # {'data_3': -287, 'data_2': -10, 'data_1': -31,
    # 'sensor_id': 271003684}}
    # Was called about 30-40 times a second in my N82 test
    if a_data.has_key('data_1'):
        global acc_data_1, acc_data_2, acc_data_3
        acc_data_1 = a_data['data_1']
        acc_data_2 = a_data['data_2']
        acc_data_3 = a_data['data_3']

    cb_handle_redraw()

def draw_box(x1, x2, a_value, a_color):
    ''' Draw a bar with sensor data '''
    if a_value == 0:
        return
    value = abs(a_value)
    if value > my_scale:
        value = my_scale
    value = value / my_zoom
    if a_value > 0:
        top = origo_y - value
        bot = origo_y
    else:
        top = origo_y
        bot = origo_y + value
    img.rectangle([(x1, top), (x2, bot)], fill=a_color)

def draw_bars():
    ''' Show all bars with sensor data '''
    if use_data_1:
        draw_box(50, 100, acc_data_1, RGB_RED)
    if use_data_2:
        draw_box(110, 160, acc_data_2, RGB_GREEN)
    if use_data_3:
        draw_box(170, 220, acc_data_3, RGB_BLUE)
    img.line(((10, origo_y), (230, origo_y)), outline=RGB_BLACK)

def draw_dot(a_data, a_color):
    if a_data < 0:
        mark = -1
        y1 = abs(a_data)
    else:
        mark = 1
        y1 = a_data
    if y1 > my_scale:
        y1 = my_scale
    y1 = origo_y + mark * (y1 / my_zoom)

    im1.point((screen_x-1, y1), width=3, outline=a_color)

def draw_graph():
    ''' Show graph that something happened '''
    if use_data_1:
        draw_dot(acc_data_1, RGB_RED)
    if use_data_2:
        draw_dot(acc_data_2, RGB_GREEN)
    if use_data_3:
        draw_dot(acc_data_3, RGB_BLUE)
    im1.point([screen_x-1, origo_y], outline=RGB_BLACK)
    img.blit(im1, target=(40,20), source=(41,20,screen_x,screen_y-50))
    im1.blit(img)

def draw_vector():
    ''' Show vectors that something happened '''
    if use_data_3:
        # horizontally on back: up-down jumps
        if acc_data_3 < 0:
            mark = -1
            a3 = abs(acc_data_3)
        else:
            mark = 1
            a3 = acc_data_3
        # Not quite right, but better than not at all
        if a3 > my_scale:
            a3 = my_scale
        # Draw with hardocoded 45 degree angle
        # cos(45 degrees) = 0.707106781
        # sin(45 degrees) = 0.707106781
        x3 = origo_x + mark * (int(a3 * 0.707106781) / my_zoom) * -1
        y3 = origo_y + mark * (int(a3 * 0.707106781) / my_zoom)
        img.line([origo_x, origo_y, x3, y3], width=10, outline=RGB_BLUE)

    if use_data_2:
        # Horizontally sideways: left-right turns
        if acc_data_2 < 0:
            mark = -1
            x3 = abs(acc_data_2)
        else:
            mark = 1
            x3 = acc_data_2
            color = RGB_GREEN
        if x3 > my_scale:
            x3 = my_scale
        x3 = origo_x + mark * (x3 / my_zoom)
        img.line([origo_x, origo_y, x3, origo_y], width=10, outline=RGB_GREEN)

    if use_data_1:
        # Standing vertically: top-bottom turns
        if acc_data_1 < 0:
            mark = -1
            y3 = abs(acc_data_1)
        else:
            mark = 1
            y3 = acc_data_1
        if y3 > my_scale:
            y3 = my_scale
        y3 = origo_y + mark * (y3 / my_zoom)
        img.line([origo_x, origo_y, origo_x, y3], width=10, outline=RGB_RED)

    x, y = canvas.size
    img.line(((0, origo_y), (x, origo_y)), outline=RGB_BLACK)  # x-coordinate axel
    img.line(((origo_x, 0), (origo_x, y)), outline=RGB_BLACK)  # y-coordinate axel

# def handle_tapped(dummy):
#     ''' My own handler for tap events '''
#     if SENSOR_TAP == False:
#         return
#     draw_turn(TURN_CENTER)

def cb_handle_redraw(dummy=(0, 0, 0, 0)):
    ''' Overwrite default screen redraw event handler '''
    if not canvas:
        return
    if img:
        # Collect statistics
        global min_data, min_data_1, min_data_2, min_data_3
        global max_data, max_data_1, max_data_2, max_data_3
        min_data_1 = min(acc_data_1, min_data_1)
        min_data_2 = min(acc_data_2, min_data_2)
        min_data_3 = min(acc_data_3, min_data_3)
        max_data_1 = max(acc_data_1, max_data_1)
        max_data_2 = max(acc_data_2, max_data_2)
        max_data_3 = max(acc_data_3, max_data_3)
        min_data = min(min_data_1, min_data_2, min_data_3)
        max_data = max(max_data_1, max_data_2, max_data_3)

        # Remove old and draw new data
        img.clear()
        img.text((5, 15), u'Max', font="dense")
        img.text((60, 15), u'%d' % (max_data_1))
        img.text((120, 15), u'%d' % (max_data_2))
        img.text((180, 15), u'%d' % (max_data_3))
        img.text((5, 40), u'%d' % my_scale)
        img.text((5, 260), u'%d' % (-1 * my_scale))
        img.text((5, 280), u'Min', font="dense")
        img.text((60, 280), u'%d' % min_data_1)
        img.text((120, 280), u'%d' % min_data_2)
        img.text((180, 280), u'%d' % min_data_3)
        img.text((5, 305), u'Now', font="dense")
        img.text((60, 305), u'%d' % acc_data_1)
        img.text((120, 305), u'%d' % acc_data_2)
        img.text((180, 305), u'%d' % acc_data_3)

        global my_view
        if my_view == VIEW_GRAPH:
            draw_graph()
        elif my_view == VIEW_VECTOR:
            draw_vector()
        else:
            # Default VIEW_BAR
            my_view = VIEW_BAR
            draw_bars()

        canvas.blit(img)

def toggle_data(a_id):
    ''' Toggle sensor data channel checking ON/OFF '''
    if a_id == 1:
        global use_data_1
        if use_data_1 == True:
            use_data_1 = False
        else:
            use_data_1 = True
    elif a_id == 2:
        global use_data_2
        if use_data_2 == True:
            use_data_2 = False
        else:
            use_data_2 = True
    elif a_id == 3:
        global use_data_3
        if use_data_3 == True:
            use_data_3 = False
        else:
            use_data_3 = True
    else:
        pass

def screen_shot():
    ''' Take a screen snapshot '''
    if DEBUG:
        im = graphics.screenshot()
        im.save(FILE_PIC)

def set_scale(a_zoom):
    ''' Control global zooming '''
    global my_zoom
    my_zoom += a_zoom
    if my_zoom > 10:
        my_zoom = 10
    elif my_zoom < 1:
        my_zoom = 1

    # Always stay within screen, regardless of screen size (MACIG)
    global my_scale
    my_scale = my_zoom * min(origo_x, origo_y)

def menu_view(a_view):
    ''' Switch display mode '''
    if DEBUG: print "menu_view: ", locals()
    global my_view
    if a_view < 10:
        my_view = my_view + a_view
        if my_view < VIEW_MIN:
            my_view = VIEW_MIN
        elif my_view > VIEW_MAX:
            my_view = VIEW_MAX
    else:
        my_view = a_view

    # Special case need cleanup
    if my_view == VIEW_GRAPH:
        im1.clear()

def menu_about():
    ''' Callback for menu item About '''
    appuifw.note(u'Sensor Analyzer v'+VERSION+'\n'+\
        'jouni.miettunen.googlepages.com\n(c) 2008 Jouni Miettunen')

def cb_quit():
    ''' About to exit, do clean-up first '''
    # Undo bind() just in case
    canvas.bind(key_codes.EKey0, None)
    canvas.bind(key_codes.EKey1, None)
    canvas.bind(key_codes.EKey2, None)
    canvas.bind(key_codes.EKey3, None)
    canvas.bind(key_codes.EKeyStar, None)
    canvas.bind(key_codes.EKeyHash, None)
    canvas.bind(key_codes.EKeyLeftArrow, None)
    canvas.bind(key_codes.EKeyRightArrow, None)

    # SENSOR cleanup
    if SENSOR_ACC:
        sensor_acc.disconnect()
    #if SENSOR_TAP:
    #    sensor_tap.disconnect()

    app_lock.signal()

#############################################################
appuifw.app.orientation = 'portrait'
appuifw.app.screen = 'full'
appuifw.app.title = u'Sensor Analyzer'
appuifw.app.exit_key_handler = cb_quit
appuifw.app.menu = [
    (u"Show Bars", lambda: menu_view(VIEW_BAR)),
    (u"Show Vectors", lambda: menu_view(VIEW_VECTOR)),
    (u"Show Graph", lambda: menu_view(VIEW_GRAPH)),
    (u"About", menu_about),
    (u"Exit", cb_quit)
    ]

canvas = appuifw.Canvas(
       redraw_callback = cb_handle_redraw)

# Calls automatically resize_callback which calls redraw_callback
# No resize_callback so have to call redraw_callback myself
appuifw.app.body = canvas
img = graphics.Image.new(canvas.size)
im1 = graphics.Image.new(canvas.size)
screen_x, screen_y = canvas.size
origo_x = screen_x / 2
origo_y = screen_y / 2
cb_handle_redraw()

# Keys are used for testing the code
canvas.bind(key_codes.EKey0, lambda: reset_data())
canvas.bind(key_codes.EKey1, lambda: toggle_data(1))
canvas.bind(key_codes.EKey2, lambda: toggle_data(2))
canvas.bind(key_codes.EKey3, lambda: toggle_data(3))
canvas.bind(key_codes.EKeyStar, lambda: set_scale(1))
canvas.bind(key_codes.EKeyHash, lambda: set_scale(-1))
canvas.bind(key_codes.EKeyLeftArrow, lambda: menu_view(-1))
canvas.bind(key_codes.EKeyRightArrow, lambda: menu_view(1))
canvas.bind(key_codes.EKey9, lambda: screen_shot())

# SENSOR init after everything else
if SENSOR:
    # returns the dictionary of available sensors
    sensors = sensor.sensors()

    # "Support is device dependent, e.g. Nokia 5500 supports
    # OrientationEventFilter and Nokia N95 supports RotEventFilter"
    # Hox: Also N95 seems to support OrientationEventFilter

    # Does this device have Accelerator Sensor
    if sensors.has_key('AccSensor'):
        SENSOR_ACC = True
        sensor_data = sensors['AccSensor']
        sensor_acc = sensor.Sensor(sensor_data['id'], sensor_data['category'])
        # Set default filter
        #sensor_acc.set_event_filter(sensor.OrientationEventFilter())
        # Set my own event callback handler
        #sensor_acc.connect(handle_orientation)
        sensor_acc.connect(handle_sensor_raw)

#     # Does this device have Tapping Sensor
#     if sensors.has_key('Tapping sensor'):
#         SENSOR_TAP = True
#         sensor_data = sensors['Tapping sensor']
#         sensor_tap = sensor.Sensor(sensor_data['id'], sensor_data['category'])
#         # Set my own event callback handler
#         sensor_tap.connect(handle_tapped)

#############################################################
app_lock = e32.Ao_lock()
app_lock.wait()

if DEBUG:
    sys.stderr.flush()
    sys.stdout.flush()
    sys.settrace(None)
    sys.setprofile(None)

    # Print debug footer
    print "-"*60
    print "Stopped ", time.ctime()
    # HOX: Trace/Profile writes to log even after last script line!
    #my_log.close()
