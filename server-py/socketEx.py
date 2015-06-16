#!/usr/bin/python
# -*- coding: cp936 -*-
def runServer(port):
    import socket
    import select
    import Queue
    serverInfo = '['+str(port)+']'
    server=('localhost',port)
    print 'listen '+serverInfo
    #SOCK_STREAM（流套接字）、SOCK_DGRAM（数据报文套接字）、AF_INET（IPv4）      
    sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    #阻塞与端口复用前后顺序可换
    sock.setblocking(False)
    #SOL_SOCKET（套接字描述符）、SO_REUSEADDR(端口复用)
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
    sock.bind(server)
    sock.listen(5)
    rlists=[]
    wlists=[]
    msg_que={}

    rlists.append(sock)
    timeout=10 
    while True:   
        rs,ws,es=select.select(rlists,wlists,rlists,timeout)  

        if not(rs or ws or es): 
            print serverInfo + 'timeout...'
            continue
        #读部分  
        for s in rs:  
            #看s是否是本机上用于监听的socket，是则接受连接，不是则接收数据  
            if s is sock:  
                conn,addr=s.accept()  
                #conn、addr分别是所接收到的socket对象和对方端口  
                print serverInfo + 'connect by',addr  
                conn.setblocking(False)  
                rlists.append(conn)  
                #使用字典将conn与一个队列相对应  
                msg_que[conn]=Queue.Queue()              
            else:  
                data =s.recv(1024)
                ####接收到数据####
                if data:  
                    print serverInfo + data
                    msg_que[s].put_nowait(data)  
                    if s not in wlists:
                        wlists.append(s)
                else:
                    #print serverInfo + 'no data in rs'
                    if s in wlists:  
                        wlists.remove(s) 
                    rlists.remove(s)
                    s.close  
                    del msg_que[s]
        #写部分             
        for  s in ws:  
            try:
                if msg_que.has_key(s):
                    msg=msg_que[s].get_nowait()
                else:
                    print serverInfo + 'cannot write to socket'
            except Queue.Empty:  
                print serverInfo + 'msg end' 
                wlists.remove(s)  
            else:
                ####回复数据####
                print serverInfo + 'reply '+msg
                s.send(msg)
                
        #异常部分      
        for s in es:
            print serverInfo + 'except',s.getpeername()  
            if s in rlists:  
                rlists.remove(s)  
            if s in wlists:  
                wlists.remove(s)          
            s.close  
            del msg_que[s]
			
        print serverInfo + 'while loop---------:'+ str(len(rlists[1:])) + '||' + str(len(wlists))


runServer(3001)

