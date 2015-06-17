 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"



int fd_Host[HOST_CLIENT_NUM];			// accepted connection fd
int fd_Slave[SLAVE_CLIENT_NUM];			// accepted connection fd
//char slave_cmd_for_fd[SLAVE_CLIENT_NUM][BUFFER_SIZE];  // USE NEW FD MACHENISM
//int slave_fd_hasbacked[SLAVE_CLIENT_NUM];
int host_conn_amount;
int slave_conn_amount;
static int connectDisconnectTimes=0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//pthread_mutex_lock(&mutex);
//pthread_mutex_unlock(&mutex);

void showclient(int isHost, int clientNum)
{
	int i;
	int client_max;
	int *client_fd;
	printf("client amount: %d\n", clientNum);
	if(isHost)
	{
		client_max=HOST_CLIENT_NUM;
		client_fd=fd_Host;
	}
	else
	{
		client_max=SLAVE_CLIENT_NUM;
		client_fd=fd_Slave;
	}
	printf("[%s]:",isHost?"HOST":"SLAVE");
	for (i = 0; i < client_max; i++)
	{
		printf("[%d]:%d ", i, client_fd[i]);
	}
	printf("\n\n");
}

void distributeCloudService(char* rcvbuf,char* backCmd)
{
    int i;
    char cmd[socketCMDnum][CMDperLenth+1];
    int cmdNum;
	char buf[BUFFER_SIZE];
	strcpy(buf,rcvbuf);
	cmdNum=(strlen(buf)+1)/(CMDperLenth+1);
    if(cmdNum>socketCMDnum)
        cmdNum=socketCMDnum;
    for(i=0;i<cmdNum;i++)
    {
        strncpy(cmd[i],buf+i*(CMDperLenth+1),CMDperLenth);
        cmd[i][CMDperLenth] = '\0';
        //printf("cmd %d is %s\n",i,cmd[i]);
    }
    
    if (!strncmp(cmd[0],WEATHER,CMDperLenth))
	{
        char backinfo[BUFFER_SIZE];
        printf("checkWeather\n\n");
        weatherCheck(cmd,backinfo);
        sprintf(backCmd,"%s:%s:%s:%s:%s",cmd[0],BACK,cmd[2],cmd[3],backinfo);
    }
    else if (!strncmp(cmd[0],STATUS,CMDperLenth) && !strncmp(cmd[2],SERVERSIDE,CMDperLenth))
    {
        if(host_conn_amount==0)
        {
            sprintf(backCmd,"%s:%s:%s:%s",cmd[0],BACK,cmd[2],NOHOST);
        }
        else
            sprintf(backCmd,"%s:%s:%s:%s",cmd[0],BACK,cmd[2],STATUSGOOD);
    }
    
    
}

void packageCmdWithFd(char* buf,int fd)
{
	char fdString[BUFFER_SIZE];
	char addPkgCmd[BUFFER_SIZE];//  package the cmd with start end and slave's fd  STRT:FFDD:<CMD>:ENDD
	bzero(fdString,BUFFER_SIZE);
    bzero(addPkgCmd,BUFFER_SIZE);
	
	if(fd<10)
		sprintf(fdString,"000%1d",fd);
	else if(fd<100)
		sprintf(fdString,"00%2d",fd);
	else if(fd<1000)
		sprintf(fdString,"0%3d",fd);
	else if(fd<10000)
		sprintf(fdString,"%4d",fd);
	else
	{
		sprintf(fdString,"4d",fd);
		printf("fd num is too big\n");
	}
	
	sprintf(addPkgCmd,"%s:%s:%s:%s",START,fdString,buf,END);
	strcpy(buf,addPkgCmd);
}

int findSlaveFdWithCmdNew(char* rcvbuf)
{
	// should optimize
    int i;
    char cmd[socketCMDnum][CMDperLenth+1];
    int cmdNum;
	char buf[BUFFER_SIZE];
	strcpy(buf,rcvbuf);
	cmdNum=(strlen(buf)+1)/(CMDperLenth+1);
    if(cmdNum>socketCMDnum)
        cmdNum=socketCMDnum;
    for(i=0;i<cmdNum;i++)
    {
        strncpy(cmd[i],buf+i*(CMDperLenth+1),CMDperLenth);
        cmd[i][CMDperLenth] = '\0';
        //printf("cmd %d is %s\n",i,cmd[i]);
    }
	memset(rcvbuf,0,BUFFER_SIZE);
	strcpy(rcvbuf,cmd[2]);
	for(i=3;i<cmdNum-1;i++)// ignore start fdnum and end
	{
		strcat(rcvbuf,":");
		strcat(rcvbuf,cmd[i]);
	}
	// should add some protect code later
    return atoi(cmd[1]);

}





void receivedBuff(int cliet_fd_index,char* rcvbuf,int isHost)
{
	int ret;
	char buf[BUFFER_SIZE];
	bzero(buf,BUFFER_SIZE);
	strcpy(buf,rcvbuf);
	pthread_mutex_lock(&mutex);
	if(isHost)
	{
		//host
        
        if (!strncmp(buf+(CMDperLenth+1)*2,STATUS,CMDperLenth)&&!strncmp(buf+(CMDperLenth+1)*5,PULSE,CMDperLenth)) // if pulse
        {
            connectDisconnectTimes=0;
            pthread_mutex_unlock(&mutex);
            return;
        }
        
		if(slave_conn_amount==0)
		{
			printf("No slave client exist\n");
			pthread_mutex_unlock(&mutex);
			return;
		}
		else
		{
			int fd;
			int i;
            
            
            if(fd = findSlaveFdWithCmdNew(buf))// buf changed
            {
                printf("Send from hostserver to slave client %d: %s\n\n",fd,buf);
                if( ret = send(fd, buf, BUFFER_SIZE, 0)< BUFFER_SIZE )		// send host back info to slave
                {
                    if(ret==-1&&errno==EAGAIN)
                    {
                    	printf("send timeout");
                    }
					else
					{
						printf("Send failed\n");
					}
                }
            }
            else
            {
                printf("Send from hostserver to slave client %d: %s\n\n",fd_Slave[0],buf);
                if( ret = send(fd_Slave[0], buf, BUFFER_SIZE, 0)< BUFFER_SIZE )		// send host back info to slave
                {
                    printf("Send failed\n");
                    //break;
                }
            }

			
		}
	}
	else
	{
		//slave
        
        if (!strncmp(buf,WEATHER,4)) 
		{
			
            char backCmd[BUFFER_SIZE];
            bzero(backCmd,BUFFER_SIZE);
            
            distributeCloudService(buf,backCmd);
            
            if( ret = send(fd_Slave[cliet_fd_index], backCmd, BUFFER_SIZE, 0)< BUFFER_SIZE )		// feed back slave cloutservice cmd
            {
                printf("Send failed\n");
                //break;
            }
			pthread_mutex_unlock(&mutex);
            return;
        }
        
        if (!strncmp(buf,STATUS,CMDperLenth)&&!strncmp(buf+(CMDperLenth+1)*2,SERVERSIDE,CMDperLenth))
        {
            
            char backCmd[BUFFER_SIZE];
            bzero(backCmd,BUFFER_SIZE);
            
            distributeCloudService(buf,backCmd);
            
            if( ret = send(fd_Slave[cliet_fd_index], backCmd, BUFFER_SIZE, 0)< BUFFER_SIZE )		// feed back slave cloudservice cmd
            {
                printf("Send failed\n");
                //break;
            }
            pthread_mutex_unlock(&mutex);
            return;
        }
        
		if(host_conn_amount!=1)
		{
			
            char backCmd[BUFFER_SIZE];
            int bufChkBitBase;
            bzero(backCmd,BUFFER_SIZE);
            printf("No host client exist\n");
            //sprintf(backCmd,"%s:NOHO:[Server]No host client exist",ERRORSTATUS);

            strcpy(backCmd,buf);
            bufChkBitBase = (CMDperLenth+1)*1;
            backCmd[bufChkBitBase] = 'N';
            backCmd[bufChkBitBase+1] = 'H';
            backCmd[bufChkBitBase+2] = 'S';
            backCmd[bufChkBitBase+3] = 'T';

            if( ret = send(fd_Slave[cliet_fd_index], backCmd, BUFFER_SIZE, 0)< BUFFER_SIZE )		// back to slave the error info
            {
                printf("Send failed\n");
                //break;
            }
			pthread_mutex_unlock(&mutex);
			return;
		}
		else
		{
			
			packageCmdWithFd(buf,fd_Slave[cliet_fd_index]);// buf changed
			
			if( ret = send(fd_Host[0], buf, BUFFER_SIZE, 0)< BUFFER_SIZE )   // send slave cmd to host
			{
				char backCmd[BUFFER_SIZE];
            	bzero(backCmd,BUFFER_SIZE);
				printf("Send failed\n");
				//break;
				sprintf(backCmd,"%s:SDHE:[Server]Send to host client error",ERRORSTATUS);
				if( ret = send(fd_Slave[cliet_fd_index], backCmd, BUFFER_SIZE, 0)< BUFFER_SIZE )		// back to slave the error info
	            {
	                printf("Send failed\n");
	                //break;
	            }
			}
			/*else
            	bindFdAndCmd(cliet_fd_index,buf);*/ // USE NEW FD MACHENISM
		}
	}
   // usleep(1000);
	pthread_mutex_unlock(&mutex);
}


void runserver(int port,int isHost)
{
	int sock_fd, new_fd; // listen on sock_fd, new connection on new_fd
	struct sockaddr_in server_addr;	// server address information
	struct sockaddr_in client_addr; // connector's address information
	socklen_t sin_size;
	int yes = 1;
	char buf[BUFFER_SIZE];
	int ret;
	int i;
	int *conn_amount;			// point to current connection amount
	int client_max;
	int *client_fd;
	struct timeval RxTxtimeout={6,0};
	bzero(buf,BUFFER_SIZE);
	
	if(isHost)
	{
		client_max=HOST_CLIENT_NUM;
		client_fd=fd_Host;
		conn_amount=&host_conn_amount;
	}
	else
	{
		client_max=SLAVE_CLIENT_NUM;
		client_fd=fd_Slave;
		conn_amount=&slave_conn_amount;
	}

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("[%s]Create Socket Failed!\n",isHost?"HOST":"SLAVE");
		return;
	}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
	{
		printf("[%s]setsockopt error\n",isHost?"HOST":"SLAVE");
		return;
	}
	if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO,(const char*)&RxTxtimeout, sizeof(RxTxtimeout)) < 0)
	{
		printf("[%s]setsockopt error\n",isHost?"HOST":"SLAVE");
		return;
	}
	if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&RxTxtimeout, sizeof(RxTxtimeout)) < 0)
	{
		printf("[%s]setsockopt error\n",isHost?"HOST":"SLAVE");
		return;
	}
	
	server_addr.sin_family = AF_INET;		 // host byte order
	server_addr.sin_port = htons(port);	 // short, network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

	if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("[%s]bind error\n",isHost?"HOST":"SLAVE");
		return;
	}

	if (listen(sock_fd, client_max) < 0)
	{
		printf("[%s]listen error\n",isHost?"HOST":"SLAVE");
		return;
	}

	printf("[%s]listen port %d\n",isHost?"HOST":"SLAVE", port);

	fd_set fdsr;
	int maxsock;
	struct timeval tv;

	(*conn_amount) = 0;
	sin_size = sizeof(client_addr);
	maxsock = sock_fd;
	while (1)
	{
		// initialize file descriptor set
		FD_ZERO(&fdsr);
		FD_SET(sock_fd, &fdsr);

		// timeout setting
		tv.tv_sec = TIMEOUT_SEC;
		tv.tv_usec = 0;

		// add active connection to fd set

		for (i = 0; i < client_max; i++)
		{
			if (client_fd[i] != 0)
			{
				FD_SET(client_fd[i], &fdsr);
			}
		}

		ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
		if (ret < 0)
		{
			printf("[%s]select error\n",isHost?"HOST":"SLAVE");
			break;
		}
		else if (ret == 0)
		{
			printf("[%s]timeout, loopback\n",isHost?"HOST":"SLAVE");
            if(isHost&&host_conn_amount)
            {
                // send pulse to host every 30s
                connectDisconnectTimes++;
                if(connectDisconnectTimes>6)
                {
                    
                    pthread_mutex_lock(&mutex);
                    printf("[HOST]client no response, close\n\n");
                    close(fd_Host[0]);
                    FD_CLR(fd_Host[0], &fdsr);
                    fd_Host[0]=0;
                    fd_Host[0]=fd_Host[host_conn_amount-1];
                    fd_Host[host_conn_amount-1]=0;
                    host_conn_amount--;
                    pthread_mutex_unlock(&mutex);
                    connectDisconnectTimes=0;
                    
                }
                else
                {
                    bzero(buf,BUFFER_SIZE);
                    sprintf(buf,"%s:%s:%s:%s:%s:%s:%s",START,"0000",STATUS,CHECK,HOSTSIDE,PULSE,END);
                    if( ret = send(fd_Host[0], buf, BUFFER_SIZE, 0)< BUFFER_SIZE )   // send slave cmd to host
                    {
                        
                        printf("Send failed\n");
                        //break;
                    }
                    bzero(buf,BUFFER_SIZE);
                }
            }
			continue;
		}

		// check every fd in the set
		for (i = 0; i < (*conn_amount); i++)
		{
			if (FD_ISSET(client_fd[i], &fdsr)) // received something
			{
                bzero(buf,BUFFER_SIZE);
				ret = recv(client_fd[i], buf, BUFFER_SIZE, 0);
				if (ret <= 0) // client error need to close
				{
					pthread_mutex_lock(&mutex);
					printf("[%s]client[%d:%d] close\n\n",isHost?"HOST":"SLAVE", i,client_fd[i]);
					close(client_fd[i]);
					FD_CLR(client_fd[i], &fdsr);
					client_fd[i]=0;
					client_fd[i]=client_fd[(*conn_amount)-1];
					client_fd[(*conn_amount)-1]=0;
					/*strcpy(slave_cmd_for_fd[i], slave_cmd_for_fd[(*conn_amount)-1]); // USE NEW FD MACHENISM
					slave_fd_hasbacked[i]=slave_fd_hasbacked[(*conn_amount)-1];
					bzero(slave_cmd_for_fd[(*conn_amount)-1],BUFFER_SIZE);
					slave_fd_hasbacked[(*conn_amount)-1]=0;*/
					(*conn_amount)--;
					pthread_mutex_unlock(&mutex);
				}
				else 
				{
					// receive data
					if (ret < BUFFER_SIZE)
						buf[ret]=0;
					printf("[%s]client[%d:%d] rcv:%s\n\n",isHost?"HOST":"SLAVE", i,client_fd[i], buf);
					receivedBuff(i,buf,isHost);
					bzero(buf,BUFFER_SIZE);
				}
			}
		}

		// check whether a new connection comes
		if (FD_ISSET(sock_fd, &fdsr))
		{
			new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
			if (new_fd <= 0)
			{
				printf("[%s]accept error\n",isHost?"HOST":"SLAVE");
				continue;
			}

			// add to fd queue
			if ((*conn_amount) < client_max)
			{
				client_fd[(*conn_amount)++] = new_fd;
				printf("[%s]new connection client[%d:%d] %s:%d\n\n",isHost?"HOST":"SLAVE" ,(*conn_amount)-1,client_fd[i], inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				if (new_fd > maxsock)
					maxsock = new_fd;
			}
			else
			{
				printf("[%s]max connections arrive, exit\n\n",isHost?"HOST":"SLAVE");
				send(new_fd, "ERRR:TMCL:Too many clients, bye", BUFFER_SIZE, 0);
				close(new_fd);
				//break;
			}
		}
		//showclient(isHost,(*conn_amount));
	}

	// close other connections
	for (i = 0; i < client_max; i++)
	{
		if (client_fd[i] != 0)
		{
			close(client_fd[i]);
		}
	}

}

void* serverHost()
{
	printf("server host\n");
	runserver(HOSTPORT,1);
	while(1);
}

void* serverSlave()
{
	printf("server slave\n");
	runserver(SLAVEPORT,0);
	while(1);
}

int main ()
{
    pthread_t thread1_id;
    pthread_t thread2_id;
    pthread_create (&thread1_id, NULL, serverHost, NULL);//connect to home
    pthread_create (&thread2_id, NULL, serverSlave, NULL);//connect to phone client

    pthread_join (thread1_id, NULL);

    pthread_join (thread2_id, NULL);

    return 0;
}
