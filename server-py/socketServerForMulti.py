#!/usr/bin/python
# -*- coding:utf-8 -*-
class RealankQueue:
    class Empty(Exception):
        pass
    def __init__(self):
        self.queue = []
    def pop(self,index=-1):
        if len(self.queue)==0 :
            raise self.Empty()
        else:
            return self.queue.pop(index)
    def put(self,item):
        self.queue.append(item)
    def __iter__(self):
        return self.queue.__iter__()

hostRlists=[]
hostWlists=[]
hostMsg_que={}

slaveRlists=[]
slaveWlists=[]
slaveMsg_que={}

uploadMsgLog={}

HOST_CLIENT_NUM = 1
SLAVE_CLIENT_NUM = 100

hostTimeoutTimes = 0

START = 'STRT'
END = "ENDD"
STATUS = "STUS"
ISOK = "ISOK"
SERVERSIDE = "SEVR"
HOSTSIDE = "HOST"
STATUSGOOD = "GOOD"
NOHOST = "NHST"
PULSE = "PLSE"
CHECK = "CHEK"
SET = "SETE"
BACK = "BACK"
START = "STRT"
END = "ENDD"
ERRORSTATUS = "ERRR"
SETERROR = "STER"
WEATHER = "WTHR"

SlaveSocketSeqNum = 0

def trimString(text):
	text = text.strip()
	start = 0
	for i in range(len(text)):
		if text[i] != '\0':
			start = i
			break
	text = text[start:]
	end = 0
	if len(text) > 0:
		for i in range(len(text)-1,0,-1):
			if text[i] != '\0':
				end = i+1
				break
		text = text[:end]
	else:
		text = None
	
	return text



def packageUploadCmd(s,cmd):
	import random
	global SlaveSocketSeqNum
	SlaveSocketSeqNum+=1
	if SlaveSocketSeqNum>90:
		SlaveSocketSeqNum = 0
	code = '%02d'%SlaveSocketSeqNum + '%02d'%random.randint(0,80)
	uploadMsgLog[code]=s
	return START+':'+ code + ':' + cmd + ':' + END
	
def unpackDownloadCmd(cmd):
	print 'unpacked: ' + str(len(cmd))+ ' : ' + cmd
	if len(cmd)>10 and cmd[0:4] == START and cmd[-4:]==END:
		code = cmd[5:9]
		print 'code is '+code
		unpackCmd = cmd[10:-5]
		if uploadMsgLog.has_key(code):
			print 'unpack dl cmd: find socket by code'
			s = uploadMsgLog[code]
			del uploadMsgLog[code]
			if s in slaveRlists:
				return (s,unpackCmd)
	print 'unpackDownloadCmd can\'t unpack'
	return (None,None)
		
def handleCloudMsgForSlave(cmd):
	'''To Judge Invalid Msg and Handle Cloud Msg'''
	cmdArray = cmd.split(':')
	if cmdArray[0]==STATUS and cmdArray[2]==SERVERSIDE:
		if len(hostRlists)>1:
			return True, STATUS+':'+BACK+':'+SERVERSIDE+':'+STATUSGOOD
		else:
			return True, STATUS+':'+BACK+':'+SERVERSIDE+':'+NOHOST
	if cmdArray == WEATHER:
		pass
	return False,None
	
def handleCloudMsgForHost(cmd):
	'''To handle pulse msg and other host cloud service in the future'''
	if cmd == START+':'+'0000'+':'+STATUS+':'+BACK+':'+HOSTSIDE+':'+PULSE+':'+END:
		global hostTimeoutTimes
		hostTimeoutTimes = 0
		return True
	return False
	
def handleUploadMessage(s,cmd):
	cmd = trimString(cmd)
	print 'handleUploadMessage:['+cmd+'] Begin====='
	ret,handledMsg = handleCloudMsgForSlave(cmd)
	if ret:
			s.send(handledMsg)
			slaveRlists.remove(s)
			s.close()
	else:
		if len(hostRlists) <= 1:
			s.send("NHST:SEVR:No host client exist")
			slaveRlists.remove(s)
			s.close()
		else:
			for host in hostRlists[1:]:
				host.send(packageUploadCmd(s,cmd))
	print 'handleUploadMessage:['+cmd+'] End====='

def handleDownloadMessage(cmd):
	cmd = trimString(cmd)
	print 'handleDownloadMessage:['+cmd+'] Begin====='
	cmdArray = cmd.split(END)[:-1]
	for i in range(0,len(cmdArray)):
		cmdArray[i] += END
	for cmd in cmdArray:
		ret = handleCloudMsgForHost(cmd)
		if ret == False:
			slave,unpackCmd=unpackDownloadCmd(cmd)
			if slave != None:
				slave.send(unpackCmd)
				slaveRlists.remove(slave)
				slave.close()
				print 'handleDownloadMessage:['+cmd+'] End====='

def runServer(port,isHost):
	import socket
	import select
	import Queue
	serverInfo = '['+str(port)+']'
	server=('182.92.183.168',port)
	print 'listen '+serverInfo
	#SOCK_STREAM(流套接字),SOCK_DGRAM(数据报文套接字),AF_INET(IPv4)
	sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	#阻塞与端口复用前后顺序可换
	sock.setblocking(False)
	#SOL_SOCKET（套接字描述符）、SO_REUSEADDR(端口复用)  
	sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
	sock.bind(server)
	sock.listen(5)
	##
	rlists=slaveRlists
	wlists=slaveWlists
	msg_que=slaveMsg_que
	limitedClients = SLAVE_CLIENT_NUM
	if isHost == True:
		rlists=hostRlists
		wlists=hostWlists
		msg_que=hostMsg_que
		limitedClients = HOST_CLIENT_NUM
	##
	rlists.append(sock)
	timeout=10 
	while True:   

		rs,ws,es=select.select(rlists,wlists,rlists,timeout)  
		print serverInfo + 'while loop---------:'+ str(len(rlists[1:])) + '||' + str(len(wlists))
		#timeout
		if not(rs or ws or es): 
			global hostTimeoutTimes
			if isHost==True:
				print serverInfo + 'timeout (hostTimeoutTimes:' + str(hostTimeoutTimes) + ') ...' 
			else:
				print serverInfo + 'timeout...'
			if isHost==True and len(rlists) > 1:
				hostTimeoutTimes+=1
				if hostTimeoutTimes>6:
					print '[HOST]client no response, close'
					for i in range(len(rlists)-1,0,-1):
						rlists[i].close()
						del rlists[i]
					#rlists = rlists[:1]#bug
					global uploadMsgLog
					for index in uploadMsgLog.keys():
						del uploadMsgLog[index]
					hostTimeoutTimes = 0
				else:
					for host in rlists[1:]:
						host.send(START+':'+'0000'+':'+STATUS+':'+CHECK+':'+HOSTSIDE+':'+PULSE+':'+END)
			if isHost==False and len(rlists) > 1:
				print '[SLAVE]clean useless sockets that not respense'
				#rlists = rlists[:1]
				for i in range(len(rlists)-1,0,-1):
					rlists[i].close()
					del rlists[i]
			continue

		#读部分  
		for s in rs:  
			#看s是否是本机上用于监听的socket，是则接受连接，不是则接收数据  
			if s is sock:  
				conn,addr=s.accept()  
				#conn、addr分别是所接收到的socket对象和对方端口  
				print serverInfo + 'connect by',addr  
				conn.setblocking(False)  
				if(len(rlists) > limitedClients):
					if isHost==False:
						print serverInfo + 'max connections arrive, exit'
						conn.send(START+':'+'ERRR:TMCL:Too many clients, bye'+END)
						conn.close()
					else:
						print serverInfo + 'max connections arrive, clear out the old, and let new in'
						old = rlists[1]
						del rlists[1]
						rlists.append(conn)
						msg_que[conn]=RealankQueue()
						old.send(START+':'+'ERRR:TMCL:Too many clients,old should be cleared, bye'+END)
						del msg_que[old]
						old.close()
				else:
					rlists.append(conn)
					#使用字典将conn与一个队列相对应  
					msg_que[conn]=RealankQueue()#Queue.Queue()              
			else:  
				data =s.recv(1024)
				####接收到数据####
				if data:  
					data = '%s'%str(data)
					print serverInfo + data
					if isHost==False:
						handleUploadMessage(s,data)
					else:
						handleDownloadMessage(data)
				else:
					#print serverInfo + 'no data in rs'
					if s in wlists:  
						wlists.remove(s) 
					rlists.remove(s)
					print serverInfo + 'close socket in rs'
					s.close()
					del msg_que[s]
		#写部分--useless
		for  s in ws:  
			try:
				if msg_que.has_key(s):
					msg=msg_que[s].pop()
				else:
					print serverInfo + 'cannot write to socket'
			except RealankQueue.Empty:  
				print serverInfo + 'msg end' 
				wlists.remove(s)  
			else:
				####回复数据####
				print serverInfo + 'reply '+msg
				s.send(msg)
				s.close()

        #异常部分      
		for s in es:
			print serverInfo + 'except',s.getpeername()  
			if s in rlists:  
				rlists.remove(s)  
			if s in wlists:  
				wlists.remove(s)
			print serverInfo + 'close socket in es'
			s.close()  
			del msg_que[s]




import threading
import time
t1 = threading.Thread(target=runServer,args=(3000,True))
t2 = threading.Thread(target=runServer,args=(3001,False))
t1.start()
time.sleep(1)
t2.start()
