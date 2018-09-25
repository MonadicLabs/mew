import socket
import time

usleep = lambda x: time.sleep(x/1000000.0)

UDP_IP = "127.0.0.1"
UDP_PORT = 9940
MESSAGE = "Hello, World!________________________________________________________________________________________________________"

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT
print "message:", MESSAGE

sock = socket.socket(socket.AF_INET, # Internet
             socket.SOCK_DGRAM) # UDP
             
while True:
	print "popo"
	sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
	usleep(10000)
