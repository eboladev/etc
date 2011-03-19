"""
Comment from the author:
This is an attempt to write a 3d engine. It is very early in development and is not yet usable.
To test it just start the script. You should be seeing a coordinate system and a spinning house.
Use the keys WASD to fly around. The up and down arrow keys get you up and down. Use the mouse like
in an ego shooter to look around.
Although this looks impressive to me as far as it goes the engine has some problems.
Try flying through the house, and the house will reappear in front instead of staying behind you.
If you look closely enough you will see that the coordinate axes do not cross in one point as they should.
To fix that the function conv_3d_to_2d needs to be improved. I would apreciate some help with the math.
Another problem is importing pictures. For that a function is needed that can stretch an image to any dimension.
(pygame.transform.scale() cannot do that)

If you have any comments, ideas or improvements mail me at "e-nomine-deus-ex@web.de".
mfg nwp.
"""

import pygame,time,math
from pygame.locals import *
from pygame import quit
pygame.init()
pygame.mouse.set_visible(False)
pygame.event.set_grab(True)
screen=pygame.display.set_mode((1024,768),HWSURFACE|DOUBLEBUF|FULLSCREEN)


global font15,objectlist
font15=pygame.font.Font(pygame.font.get_default_font(),15)
objectlist=[]

def conv_3d_to_2d((x,y,z)):
    """convert 3d coordinates to 2d coordinates"""
    try:
        x,y,z=move((camera.x,camera.y,camera.z),(-x,-y,-z))
        distance=(x**2+y**2+z**2)**.5
        zoom1=512*(180./camera.zoom)
        zoom2=384*(180./camera.zoom)
        hight=math.radians(camera.hight)
        direction=math.radians(camera.direction)
        #Critical function. Please improve.
        dx=(-x*math.sin(direction)\
            -y*math.cos(direction))/distance*zoom1
        dy=(-z*math.cos(hight)-\
            x*math.sin(hight)*math.cos(direction)+\
            y*math.sin(hight)*math.sin(direction))/distance*zoom2

        return 512+dx,384-dy
    except ZeroDivisionError:
        return 512,384


class camera:
    x=1000
    y=50
    z=70
    zoom=90
    direction=0
    hight=0

def move((x,y,z),(x1,y1,z1)):
    """Moves a point x,y,z relatively in direction x1,y1,z1"""
    return x+x1,y+y1,z+z1

def sortlist():
    """Sort the objectlist by distance to the camera so that close objects will cover distant objects"""
    for object in objectlist:
        if object[1]=="point":
            x,y,z=object[2]
            object[0]=((x-camera.x)**2+((y-camera.y)**2+(z-camera.z)**2)**.5)
        elif object[1]=="line":
            a=object[2]
            b=object[3]
            object[0]=(((a[0]+b[0])/2-camera.x)**2+((a[1]+b[1])/2-camera.y)**2+((a[2]+b[2])/2-camera.z)**2)**.5
        elif object[1]=="sphere":
            x,y,z=object[2]
            radius=object[3]
            object[0]=((x-camera.x)**2+(y-camera.y)**2+(z-camera.z)**2)**.5-radius
        elif object[1]=="writing":
            pass
        elif object[1]=="polygon":
            liste=object[2]
            k=0
            x=0
            y=0
            z=0
            while k<len(liste):
                x+=liste[k][0]
                y+=liste[k][1]
                z+=liste[k][2]
                k+=1
            object[0]=((x/k-camera.x)**2+(y/k-camera.y)**2+(z/k-camera.z)**2)**.5
        objectlist.sort()
        objectlist.reverse()

def write((x,y),text,font,color):
    """write text on screen"""
    screen.blit(font.render(text,True,color),(x,y))
#objects
def point((x,y,z),color):
    objectlist.append([0,"point",(x,y,z),color])
def line(a,b,color):
    objectlist.append([0,"line",a,b,color])
def sphere((x,y,z),radius,color):
    objectlist.append([0,"sphere",(x,y,z),radius,color])
def polygon(liste,color):
    objectlist.append([0,"polygon",liste,color])

def draw():#draw objects
    """draws objects"""
    sortlist()
    screen.lock()
    global objectlist
    for object in objectlist:
        if object[1]=="writing":
            schrift=object[2]
            x,y=object[3]
            screen.unlock()
            screen.blit(schrift,(x,y))
            screen.lock()
        elif object[1]=="point":
            point=object[2]
            color=object[3]
            screen.set_at(conv_3d_to_2d(point),color)
        elif object[1]=="line":
            a=object[2]
            b=object[3]
            color=object[4]
            pygame.draw.line(screen,color,conv_3d_to_2d(a),conv_3d_to_2d(b),1)
        elif object[1]=="sphere":
            x,y,z=object[2]
            radius=object[3]
            color=object[4]
            distance=((x-camera.x)**2+(y-camera.y)**2+(z-camera.z)**2)**.5
            zoom=250*(180./camera.zoom)
            pygame.draw.circle(screen,color,conv_3d_to_2d((x,y,z)),radius/distance*zoom,0)
        elif object[1]=="polygon":
            liste=object[2]
            k=0
            while k<len(liste):
                liste[k]=conv_3d_to_2d(liste[k])
                k+=1
            color=object[3]
            pygame.draw.polygon(screen,color,liste,0)
    screen.unlock()
    objectlist=[]


def input(events):
    pygame.event.pump()
    if pygame.key.get_pressed()[270]==1:
        gamma+=.1
        pygame.display.set_gamma(gamma)
    if pygame.key.get_pressed()[269]==1:
        gamma-=.1
        pygame.display.set_gamma(gamma)
    if pygame.key.get_pressed()[27]==1:
        quit()
    if pygame.key.get_pressed()[119]==1:#W
        camera.x-=10*math.cos(math.radians(camera.direction))*math.cos(math.radians(camera.hight))
        camera.y+=10*math.sin(math.radians(camera.direction))*math.cos(math.radians(camera.hight))
        camera.z+=10*math.sin(math.radians(camera.hight))
    if pygame.key.get_pressed()[115]==1:#S
        camera.x+=10*math.cos(math.radians(camera.direction))*math.cos(math.radians(camera.hight))
        camera.y-=10*math.sin(math.radians(camera.direction))*math.cos(math.radians(camera.hight))
        camera.z-=10*math.sin(math.radians(camera.hight))
    if pygame.key.get_pressed()[100]==1:#D
        camera.x+=10*math.sin(math.radians(camera.direction))
        camera.y+=10*math.cos(math.radians(camera.direction))
    if pygame.key.get_pressed()[97]==1:#A
        camera.x-=10*math.sin(math.radians(camera.direction))
        camera.y-=10*math.cos(math.radians(camera.direction))
    if pygame.key.get_pressed()[273]==1:#up
        camera.z+=10
    if pygame.key.get_pressed()[274]==1:#down
        camera.z-=10
    if pygame.key.get_pressed()[101]==1:#Q
        camera.zoom-=5
        if camera.zoom<5:
            camera.zoom=5
    if pygame.key.get_pressed()[113]==1:#E
        camera.zoom+=5
    x,y=pygame.mouse.get_rel()
    camera.direction+=x/30.
    if camera.direction>360:
        camera.direction-=360
    if camera.direction<0:
        camera.direction+=360
    camera.hight-=y/30.
    if camera.hight>90:
        camera.hight=90
    if camera.hight<-90:
        camera.hight=-90

#data
def coordinate_system():
    global font15
    write(conv_3d_to_2d((200,0,0)),"X",font15,(0,0,0))
    line((-200,0,0),(200,0,0),(0,0,0))
    polygon([(200,0,0),(195,5,0),(195,0,5)],(50,50,50))
    polygon([(200,0,0),(195,5,0),(195,0,-5)],(0,0,0))
    polygon([(200,0,0),(195,-5,0),(195,0,5)],(100,100,100))
    polygon([(200,0,0),(195,-5,0),(195,0,-5)],(25,25,25))
    write(conv_3d_to_2d((0,200,0)),"Y",font15,(0,0,0))
    line((0,-200,0),(0,200,0),(0,0,0))
    polygon([(0,200,0),(5,195,0),(0,195,5)],(50,50,50))
    polygon([(0,200,0),(5,195,0),(0,195,-5)],(0,0,0))
    polygon([(0,200,0),(-5,195,0),(0,195,5)],(100,100,100))
    polygon([(0,200,0),(-5,195,0),(0,195,-5)],(25,25,25))
    write(conv_3d_to_2d((0,0,200)),"Z",font15,(0,0,0))
    line((0,0,-200),(0,0,200),(0,0,0))
    polygon([(0,0,200),(5,0,195),(0,5,195)],(50,50,50))
    polygon([(0,0,200),(5,0,195),(0,-5,195)],(0,0,0))
    polygon([(0,0,200),(-5,0,195),(0,5,195)],(100,100,100))
    polygon([(0,0,200),(-5,0,195),(0,-5,195)],(25,25,25))

def house():
    now=time.time()
    a=-math.cos(now)*50,-math.sin(now)*50,0
    b=-math.cos(now+math.pi*.5)*50,-math.sin(now+math.pi*.5)*50,0
    c=-math.cos(now+math.pi)*50,-math.sin(now+math.pi)*50,0
    d=-math.cos(now+math.pi*1.5)*50,-math.sin(now+math.pi*1.5)*50,0
    e=-math.cos(now)*50,-math.sin(now)*50,100
    f=-math.cos(now+math.pi*.5)*50,-math.sin(now+math.pi*.5)*50,100
    g=-math.cos(now+math.pi)*50,-math.sin(now+math.pi)*50,100
    h=-math.cos(now+math.pi*1.5)*50,-math.sin(now+math.pi*1.5)*50,100

    polygon([a,b,c,d],(255,0,255))#ground
    polygon([e,f,g,h],(0,255,0))#top
    #polygon([a,b,f,e],(255,255,0))#front
    polygon([c,d,h,g],(255,255,51))#back
    polygon([b,c,g,f],(255,255,102))#right
    polygon([a,d,h,e],(255,255,153))#left
    #roof
    i=0,0,150
    polygon([e,f,i],(255,0,0))
    polygon([g,h,i],(255,51,51))
    polygon([f,g,i],(255,102,102))
    polygon([h,e,i],(255,153,153))

#Mainloop
counter=pygame.time.Clock()
while True:
    screen.fill((255,255,255))  #clear screen
    house()                     #define house
    coordinate_system()         #define coordinate system
    draw()                      #draw objects
    counter.tick()              #counter for framerate
    global font15
    write((0,0),"x:"+str(int(camera.x))+" y:"+str(int(camera.y))+" z:"+str(int(camera.z))+\
            " zoom:"+str(camera.zoom)+" direction:"+str(int(camera.direction*10)/10.)+"*"+\
             " hight:"+str(int(camera.hight*10)/10.)+"*"+" FPS:"+str(int(counter.get_fps()*10)/10.),font15,(0,0,0))
    pygame.display.flip()
    input(pygame.event.get())
quit()
