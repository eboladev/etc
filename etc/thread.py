import threading

class Some:
	def run(self):
		print "running"
		print "self.value is %d" % self.value

	def __init__(self):
		self.value = 42
		th = threading.Thread(target = self.run)
		th.start()
		th.join()

def main():
	s = Some()

if __name__ == "__main__":
	main()
