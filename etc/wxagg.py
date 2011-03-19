# Copyright (C) 2005 Aver Development Corporation. All Rights Reserved.

from __future__ import division

import aggdraw
import colorsys
import math
from sets import Set
import time
import wx

def aggBrush(wxBrush):
    if wxBrush.GetStyle() == wx.TRANSPARENT:
        return aggdraw.Brush('Black', 0)
    else:
        colour = wxBrush.GetColour()
        return aggdraw.Brush((colour.Red(), colour.Green(), colour.Blue()))


def aggPen(wxPen):
    if wxPen.GetStyle() == wx.TRANSPARENT:
        return aggdraw.Pen('Black', 1, 0)
    else:
        colour = wxPen.GetColour()
        return aggdraw.Pen((colour.Red(), colour.Green(), colour.Blue()), wxPen.GetWidth() or 1)


class AggDC:
    PassThrough = Set(
                      '''
                         BeginDrawing
                         EndDrawing
                         GetBackground
                         GetSize
                         GetSizeTuple
                         SetBrush
                         SetPen
                      '''.split()
                     )

    def __init__(self, dc):
        self.dc = dc
        self.dc.BeginDrawing()
        # w, h = self.dc.GetSizeTuple()
        w = 400
        h = 300
        self.draw = aggdraw.Draw('RGB', (w, h))
        # self.draw.rectangle((0, 0, w, h), None, aggBrush(dc.GetBackground()))

    def __del__(self):
        # w, h = self.dc.GetSizeTuple()
        w = 400
        h = 300
        if w and h:
            image = wx.EmptyImage(w, h)
            image.SetData(self.draw.tostring())
            self.dc.DrawBitmap(image.ConvertToBitmap(), 0, 0)
        self.dc.EndDrawing()

    def __getattr__(self, attr):
        if attr in self.PassThrough:
            return getattr(self.dc, attr)
        else:
            raise AttributeError("%s instance has no attribute '%s'" % (self.__class__.__name__, attr))


    def CrossHair(self, x, y):
        self.dc.CrossHair(x, y)

        w, h = self.dc.GetSizeTuple()
        p = aggPen(self.dc.GetPen())
        self.draw.line((0, y, w, y), p)
        self.draw.line((x, 0, x, h), p)


    def DrawArc(self, x1, y1, x2, y2, xc, yc):
        self.dc.DrawArc(x1, y1, x2, y2, xc, yc)

        b = aggBrush(self.dc.GetBrush())
        p = aggPen(self.dc.GetPen())
        radius = ((xc-x1)**2 + (yc-y1)**2)**0.5
        self.draw.pieslice(
                           (xc-radius, yc-radius, xc+radius, yc+radius),
                           math.degrees(math.atan2(yc-y1, x1-xc)),
                           math.degrees(math.atan2(yc-y2, x2-xc)),
                           p, b
                          )


    def DrawCircle(self, x, y, radius):
        self.dc.DrawCircle(x, y, radius)

        b = aggBrush(self.dc.GetBrush())
        p = aggPen(self.dc.GetPen())
        self.draw.ellipse((x-radius, y-radius, x+radius, y+radius), p, b)


    def DrawEllipse(self, x, y, width, height):
        self.dc.DrawEllipse(x, y, width, height)

        b = aggBrush(self.dc.GetBrush())
        p = aggPen(self.dc.GetPen())
        self.draw.ellipse((x, y, x+width, y+height), p, b)


    def DrawLine(self, x1, y1, x2, y2):
        self.dc.DrawLine(x1, y1, x2, y2)

        p = aggPen(self.dc.GetPen())
        self.draw.line((x1, y1, x2, y2), p)


if __name__ == '__main__':
    class MyPanel(wx.Panel):
        def __init__(self, parent, agg):
            wx.Panel.__init__(self, parent, style=wx.WANTS_CHARS)
            self.agg = agg
            self.SetBackgroundColour('White')
            self.Bind(wx.EVT_PAINT, self.OnPaint)
            self.Bind(wx.EVT_TIMER, self.OnTimer)
	    self.w = 400
            self.h = 300
            self.dir = -3
            self.cnt = 0
	    self.timer = wx.Timer(self)
            self.c1 = math.cos(math.radians(22.5))
            self.c2 = math.cos(math.radians(67.5))
            self.s1 = math.sin(math.radians(22.5))
	    self.s2 = math.sin(math.radians(67.5))
            self.timeout = 1000/50
            self.timer.Start(self.timeout)

        def OnTimer(self, evt):
	    self.w += self.dir
            self.h += self.dir
            self.cnt += abs(self.dir)
            if self.cnt >= 300:
                self.cnt = 0
                self.dir = -self.dir
	    self.RefreshRect(wx.Rect(0, 0, 400, 300), False)

        def OnPaint(self, event):
            dc = AggDC(wx.PaintDC(self))
            w = self.w
            h = self.h
            dc.BeginDrawing()
            dc.DrawLine(0, 0, w, h)
            dc.SetBrush(wx.BLUE_BRUSH)
            dc.SetPen(wx.RED_PEN)
            dc.DrawArc(
                       w/4 - w*self.c1/4, h/4 - w*self.s1/4,
                       w/4 - w*self.c2/4, h/4 - w*self.s2/4,
                       w/4, h/4
                      )
            dc.DrawCircle(3*w/4, 3*h/4, min(w/4, h/4))
            dc.SetPen(wx.Pen('Red', 5))
            dc.DrawEllipse(0, h/2, w/2, h/2)
            x = 3*w/4
            y = h/4
            r = 4*min(w/4, h/4)/10
            dc.SetPen(wx.TRANSPARENT_PEN)
            for a in range(0, 360, 20):
                red, green, blue = colorsys.hsv_to_rgb(a/360, 1, 1)
                dc.SetBrush(wx.Brush((255*red, 255*green, 255*blue)))
                dc.DrawCircle(x+1.2*r*math.cos(math.radians(a)), y+1.2*r*math.sin(math.radians(a)), r)
            dc.EndDrawing()


    class MyFrame(wx.Frame):
        def __init__(self, parent, title):
            wx.Frame.__init__(self, parent, -1, title, style=wx.DEFAULT_FRAME_STYLE | wx.FULL_REPAINT_ON_RESIZE)
            panel = wx.Panel(self)
            sizer = wx.BoxSizer(wx.HORIZONTAL)
            sizer.Add(MyPanel(panel, True), 10, wx.ALL | wx.EXPAND, 7)
            # sizer.Add(MyPanel(panel, False), 10, wx.ALL | wx.EXPAND, 7)
            panel.SetSizer(sizer)
            panel.Layout()


    class MyApp(wx.App):
        def OnInit(self):
            frame = MyFrame(None, 'AGG Tester')
            self.SetTopWindow(frame)
            frame.Show()
            return True

    app = MyApp(redirect=False)
    app.MainLoop()
