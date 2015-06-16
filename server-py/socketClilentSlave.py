import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'prepare to connect to server'
sock.connect(('localhost', 3001))
import time
#time.sleep(2)
sock.send('STUS:CHEK:SEVR')
print sock.recv(1024)
sock.close()


