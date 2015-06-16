#!/usr/bin/python
# -*- coding: cp936 -*-
def runServer(port):
    import socket
    import select
    import Queue
    serverInfo = '['+str(port)+']'
    server=('localhost',port)
    print 'listen '+serverInfo
    #SOCK_STREAM�����׽��֣���SOCK_DGRAM�����ݱ����׽��֣���AF_INET��IPv4��      
    sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    #������˿ڸ���ǰ��˳��ɻ�
    sock.setblocking(False)
    #SOL_SOCKET���׽�������������SO_REUSEADDR(�˿ڸ���)
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
        #������  
        for s in rs:  
            #��s�Ƿ��Ǳ��������ڼ�����socket������������ӣ��������������  
            if s is sock:  
                conn,addr=s.accept()  
                #conn��addr�ֱ��������յ���socket����ͶԷ��˿�  
                print serverInfo + 'connect by',addr  
                conn.setblocking(False)  
                rlists.append(conn)  
                #ʹ���ֵ佫conn��һ���������Ӧ  
                msg_que[conn]=Queue.Queue()              
            else:  
                data =s.recv(1024)
                ####���յ�����####
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
        #д����             
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
                ####�ظ�����####
                print serverInfo + 'reply '+msg
                s.send(msg)
                
        #�쳣����      
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

