# AiO Torch
# S60 Strobe Light Pro 
# feat. Screen Torch
#
# By T M -- http://series-sixty.blogspot.com
#


from appuifw import *
import camera, appuifw

list=[u"ScreenTorch", u"StrobeLight Pro", u"About"]

picked=appuifw.popup_menu(list, u"Select an option:")

if picked==0:
	def light_on():
  	        import miso, e32
  		miso.reset_inactivity_time()
  		e32.ao_sleep(10, light_on)

	import TopWindow, graphics, appuifw, e32
 
	height = 320
	width = 240
	top = 0
	left = 0
	window = TopWindow.TopWindow()
	image = graphics.Image.new((width,height))
	window.add_image(image, (width,height))
	window.size = (width, height)
	window.corner_type = 'square'
	window.position = (left,top)
	window.background_color = 0xFFFFFF
 
	def exit():
		window.hide()
		app_lock.signal()
 
	app_lock = e32.Ao_lock()
	window.show()
	light_on()
 	
	appuifw.app.exit_key_handler = exit
	app_lock.wait()
	appuifw.note(u"For updates and more info, visit series-sixty.blogspot.com","conf")

if picked==1:
	running=1
	def quit():
    		global running
		appuifw.note(u"StrobeLight OFF")
		appuifw.note(u"For updates and more info, visit series-sixty.blogspot.com","conf")
    		running=0
	appuifw.app.exit_key_handler=quit
	appuifw.note(u"StrobeLight ON")

	while running:
		image = camera.take_photo(exposure='night', flash='red_eye_reduce')

if picked==2:
	appuifw.note(u"ScreenTorch blanks the screen and makes the backlight stay on until it is manually terminated") 
	appuifw.note(u"StrobeLight Pro utilizes the camera hardware to continuously flash the LED at regular intervals")
	appuifw.note(u"For updates and more info, visit series-sixty.blogspot.com")