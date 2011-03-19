#/usr/bin/env python

# Shadows is PUBLIC DOMAIN and made by John Eriksson 2006 - http://arainyday.se

import math
import pygame
from pygame.locals import *

class Shadows:
        
    LIGHT_SIZE = 100

    mapd = []    
    lightPos = [300,300]
    lightRect = None
    lightBody = pygame.Rect(0,0,6,6)
    
    def tracePoint(self,x1,y1,x2,y2,l):
        theta = math.atan2((y2-y1),(x2-x1));    
        if theta<0:
            d= (180*(theta+(math.pi*2))/math.pi)
        else:
            d= (180*(theta)/math.pi)
        dx = math.cos(math.radians(d))
        dy = math.sin(math.radians(d))                
    
        return (x2+dx*l,y2+dy*l)

    def getPolygon(self,x,y,box):
    
        r = box.right
        l = box.left
        t = box.top
        b = box.bottom
        L = self.LIGHT_SIZE+10
        
        tracePoint = self.tracePoint
            
        box = pygame.Rect(l,t,box.width-1,box.height-1)
        
        lightPos = (self.LIGHT_SIZE,self.LIGHT_SIZE)
               
        if x >= l and x <= r:
            if y >= b: # directly under
                #print "UNDER"
                tp1 = tracePoint(x,y,l,b,L)
                tp2 = tracePoint(x,y,r,b,L)
                return ((box.bottomleft,tp1,[lightPos[0]-L,lightPos[1]-L],[lightPos[0]+L,lightPos[1]-L],tp2,box.bottomright))
            else:   # directly above             
                #print "ABOVE"
                tp1 = tracePoint(x,y,l,t,L)
                tp2 = tracePoint(x,y,r,t,L)
                return ((box.topleft,tp1,[lightPos[0]-L,lightPos[1]+L],[lightPos[0]+L,lightPos[1]+L],tp2,box.topright))
        elif y >= t and y <= b:
            if x <= l: # directly to the left
                #print "LEFT"
                tp1 = tracePoint(x,y,l,b,L)
                tp2 = tracePoint(x,y,l,t,L)
                return ((box.bottomleft,tp1,[lightPos[0]+L,lightPos[1]+L],[lightPos[0]+L,lightPos[1]-L],tp2,box.topleft))
            else:   # directly to the right
                #print "RIGHT"
                tp1 = tracePoint(x,y,r,b,L)
                tp2 = tracePoint(x,y,r,t,L)
                return ((box.bottomright,tp1,[lightPos[0]-L,lightPos[1]+L],[lightPos[0]-L,lightPos[1]-L],tp2,box.topright))
        if y <= t:
            if x <= l: # upper left
                #print "UPPER LEFT"
                tp1 = tracePoint(x,y,r,t,L)
                tp2 = tracePoint(x,y,l,b,L)
                return ((box.topleft,box.topright,tp1,tp2,box.bottomleft))
            else:     # upper right
                #print "UPPER RIGHT"
                tp1 = tracePoint(x,y,l,t,L)
                tp2 = tracePoint(x,y,r,b,L)
                return ((box.topright,box.topleft,tp1,tp2,box.bottomright))
        elif y >= b:
            if x <= l: # lower left
                #print "LOWER LEFT"
                tp1 = tracePoint(x,y,r,b,L)
                tp2 = tracePoint(x,y,l,t,L)
                return ((box.bottomleft,box.bottomright,tp1,tp2,box.topleft))
            else:     # lower right
                #print "LOWER RIGHT"
                tp1 = tracePoint(x,y,l,b,L)
                tp2 = tracePoint(x,y,r,t,L)
                return ((box.bottomright,box.bottomleft,tp1,tp2,box.topright))
                
        return None      

    def drawMask(self,img):
        nrects = []
        for r in self.mapd:
            if self.lightRect.colliderect(r):
                nr = r.clip(self.lightRect)
                nr.top = nr.top - self.lightRect.top
                nr.left = nr.left - self.lightRect.left
                nrects.append(nr)
        
        img.fill(1)
        pygame.draw.circle(img, 2, (self.LIGHT_SIZE,self.LIGHT_SIZE), self.LIGHT_SIZE,0)
       
        for r in nrects:
            p = self.getPolygon(self.LIGHT_SIZE,self.LIGHT_SIZE,r)                
            if p:
                pygame.draw.polygon(img, 1, p, 0)
    
        pygame.draw.circle(img, 3, (self.LIGHT_SIZE,self.LIGHT_SIZE), self.lightBody.width/2,0)

    def drawMap(self,img):
        img.fill((100,100,100))
        for r in self.mapd:
            pygame.draw.rect(img, (0,0,0), r, 0)
   
    
    def moveLight(self,dx,dy):
            
        self.lightBody.centerx = self.lightPos[0]+dx
        self.lightBody.centery = self.lightPos[1]+dy
            
        for r in self.mapd:
            if r.colliderect(self.lightBody):
                return
                
        self.lightPos[0] += dx
        self.lightPos[1] += dy
        self.lightRect.center = self.lightPos
   
    def main(self):

        # init pygame
        pygame.init()

        # setup the display
        screen = pygame.display.set_mode((800, 480),FULLSCREEN | HWSURFACE)
        pygame.display.set_caption("Draw obstacles with mouse. Move light with arrow keys.")
        
        background = pygame.Surface((800,480),FULLSCREEN | HWSURFACE)
        background = background.convert()
        self.drawMap(background)
    
        mask = pygame.Surface([self.LIGHT_SIZE*2,self.LIGHT_SIZE*2],HWSURFACE|HWPALETTE,8)
        # mask.set_palette([[0,0,0],[255,0,0],[180,180,180],[255,255,255]])
        mask.set_colorkey(1, RLEACCEL)
        self.lightRect = mask.get_rect()
        self.lightRect.center = self.lightPos
    
        self.drawMask(mask)
        
        clock = pygame.time.Clock()
       
        paint = 0
        
        newRect = None
       
        dx = 0
        dy = 0
       
        while 1:
            clock.tick(30) 
    
            for event in pygame.event.get():
                if event.type == QUIT:
                    return
                elif event.type == KEYDOWN:
                    if event.key == K_ESCAPE:
                        return
                    elif event.key == K_UP:
                        dy=-2
                    elif event.key == K_DOWN:
                        dy=2
                    elif event.key == K_LEFT:
                        dx=-2
                    elif event.key == K_RIGHT:
                        dx=2
                elif event.type == KEYUP:
                    if event.key == K_UP and dy<0:
                        dy=0
                    elif event.key == K_DOWN and dy>0:
                        dy=0
                    elif event.key == K_LEFT and dx<0:
                        dx=0
                    elif event.key == K_RIGHT and dx>0:
                        dx=0
                elif event.type == MOUSEBUTTONUP:
                    if event.button==1:
                        if newRect:
                            r = pygame.Rect(newRect)
                            r.normalize()
                            newRect = None
                            if not r.collidepoint(self.lightPos) and r.width > 1 and r.height > 1:
                                self.mapd.append(r)
                                self.drawMap(background)
                                self.drawMask(mask)
                elif event.type == MOUSEBUTTONDOWN:
                    if event.button==1:
                        newRect = [event.pos[0],event.pos[1],0,0]
                elif event.type == MOUSEMOTION:
                    if event.buttons[0]:
                        newRect[2] = event.pos[0] - newRect[0]
                        newRect[3] = event.pos[1] - newRect[1]
            if dx or dy:
                self.moveLight(dx,dy)
                self.drawMask(mask)
                                   
            screen.blit(background, (0,0))
            screen.blit(mask, self.lightRect.topleft)
    
            if newRect:
                pygame.draw.rect(screen, (0,0,0), newRect, 0)
         
            pygame.display.flip()
    
def main():
    s = Shadows()
    s.main()                
     
if __name__ == '__main__': 
    main()
            
