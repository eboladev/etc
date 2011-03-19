from wxPython.glcanvas import wxGLCanvas
from wxPython.wx import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import sys,math

name = 'ball_glut'

class myGLCanvas(wxGLCanvas):
    def __init__(self, parent):
        wxGLCanvas.__init__(self, parent,-1)
        EVT_PAINT(self, self.OnPaint)
        EVT_SIZE(self, self.OnSize)
        self.init = 0
        return


    def OnSize(self, event):
        size = self.GetClientSize()
        if self.GetContext():
            self.SetCurrent()
            glViewport(0, 0, size.width, size.height)
        event.Skip()


    def OnPaint(self,event):
        dc = wxPaintDC(self)
        self.SetCurrent()
        if not self.init:
            self.InitGL()
            self.init = 1
        self.OnDraw()
        return

    def OnDraw(self):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glPushMatrix()
        color = [1.0,0.,0.,0.5]
        glMaterialfv(GL_FRONT,GL_DIFFUSE,color)
        glutSolidSphere(2,40,40)
        glPopMatrix()


        glPushMatrix()
        color = [0.,0.,1.0,0.5]
        glMaterialfv(GL_FRONT,GL_DIFFUSE,color)
        glTranslatef(1.0,0.0,0.0)
        glutSolidSphere(1.5,40,40)
        glPopMatrix()


        glPushMatrix()
        color = [0.,1.0,0.,0.5]
        glMaterialfv(GL_FRONT,GL_DIFFUSE,color)
        glTranslatef(0.5,0.5,0.0)
        glutSolidSphere(1.5,40,40)
        glPopMatrix()


        self.SwapBuffers()
        return
    
    def InitGL(self):
        # set viewing projection
        light_diffuse = [1.0, 1.0, 1.0, 1.0]
        light_position = [1.0, 1.0, 1.0, 0.0]

        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse)
        glLightfv(GL_LIGHT0, GL_POSITION, light_position)

        glEnable(GL_LIGHTING)
        glEnable(GL_LIGHT0)
        glEnable(GL_DEPTH_TEST)

        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        
        glClearColor(0.0, 0.0, 0.0, 1.0)
        glClearDepth(1.0)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(40.0, 1.0, 1.0, 30.0)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        gluLookAt(0.0, 0.0, 10.0,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0)
        return
    

def main():
    app = wxPySimpleApp()
    frame = wxFrame(None,-1,'ball_wx',wxDefaultPosition,wxSize(400,400))
    canvas = myGLCanvas(frame)
    frame.Show()
    app.MainLoop()

if __name__ == '__main__': main()
