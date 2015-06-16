import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('localhost', 8003))
sock.listen(5)
while True:
    connection,address = sock.accept()
    try:
        connection.settimeout(5)
        print 'ready to serve'
        buf = connection.recv(1024)
        if buf == '1':
            connection.send('welcome to server!')
        else:
            connection.send('please go out!')
    except socket.timeout:
        print 'time out'
    connection.close()
