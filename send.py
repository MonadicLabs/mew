import socket
import time

usleep = lambda x: time.sleep(x/1000000.0)

UDP_IP = "127.0.0.1"
UDP_PORT = 9940
MESSAGE = "hello_"

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT
print "message:", MESSAGE

sock = socket.socket(socket.AF_INET, # Internet
             socket.SOCK_DGRAM) # UDP
           
i=0  
while True:
    curMessage = MESSAGE
    curMessage = curMessage + chr(i) + chr(i) + chr(i)
    print(i)
    sock.sendto(curMessage, (UDP_IP, UDP_PORT))
    i=i+1
    if i > 255:
        i=0
    usleep(1000)
