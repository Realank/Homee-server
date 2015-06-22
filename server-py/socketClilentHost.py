import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'prepare to connect to server'
sock.connect(('182.92.183.168', 3000))
import time
#time.sleep(2)
rcv=sock.recv(1024)
while rcv:
    print 'host rcv:'+rcv
    time.sleep(2)
    sock.send('[host]:'+rcv)
    rcv=sock.recv(1024)
print 'close'
sock.close()
