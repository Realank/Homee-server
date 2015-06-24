#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <sys/select.h>       /* select function*/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "server.h"

static int connectDisconnectTimes=0;


int handle(int sockfd);


int delayedCtl(char* device, char* sec)
{
    int delayedTime=atoi(sec);
    if(!strcmp(device,FLOWERWATER))
    {
        //send FLOWERWATER
    }
    else if(!strcmp(device,PETFOOD))
    {
        //send PETFOOD
    }
    return 0;
}

int IRCtl(char* device, char* btn)
{
    int btnNum=atoi(btn);
    if(!strcmp(device,TV))
    {
        //send btnNum
    }
    else if(!strcmp(device,AIRCONDITIONOR))
    {
        //send btnNum
    }
    return 0;
}

int lightCtl(char* area, char* isCheck, char* setValue)
{
    static int livingRmLightStatus=0;
    static int bedRm01LightStatus=0;
    static int bedRm02LightStatus=0;
    static int bedRm03LightStatus=0;
    int check=0;
    int val=0;
    if(!strcmp(isCheck,CHECK))
        check=1;
    val=atoi(setValue);
    
    if(!strcmp(area,LIVINGROOM))//area
    {
        if(!check)
            livingRmLightStatus=val;
        return livingRmLightStatus;
    }
    else if(!strcmp(area,BEDROOM01))//area
    {
        if(!check)
            bedRm01LightStatus=val;
        return bedRm01LightStatus;
    }
    else if(!strcmp(area,BEDROOM02))//area
    {
        if(!check)
            bedRm02LightStatus=val;
        return bedRm02LightStatus;
    }
    else if(!strcmp(area,BEDROOM03))//area
    {
        if(!check)
            bedRm03LightStatus=val;
        return bedRm03LightStatus;
    }
    return -1;
}

int distributeHomeService(char* buf,char* backCmd)
{
    int i;
    char seperateCmd[socketCMDnum][CMDperLenth+1];
    char processedCmd[BUFFER_SIZE];
    int cmdNum=(strlen(buf)+1)/(CMDperLenth+1);
    bzero(processedCmd,BUFFER_SIZE);
    bzero(backCmd,BUFFER_SIZE);
    if(cmdNum>socketCMDnum)
        cmdNum=socketCMDnum;
    for(i=0;i<cmdNum;i++)
    {
        strncpy(seperateCmd[i],buf+i*(CMDperLenth+1),CMDperLenth);
        seperateCmd[i][CMDperLenth] = '\0';
        //printf("cmd %d is %s\n",i,seperateCmd[i]);
    }
    
    if(!strcmp(seperateCmd[2],LIGHT))
    {
        int ret = lightCtl(seperateCmd[4],seperateCmd[3],seperateCmd[5]);
        if(ret>=0)
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:000%1d:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,seperateCmd[4],ret,END);
        }
        else
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,seperateCmd[4],SETERROR,END);
        }
    }
    
    else if(!strcmp(seperateCmd[2],TV)|| !strcmp(seperateCmd[2],AIRCONDITIONOR))
    {
        int ret = IRCtl(seperateCmd[2],seperateCmd[4]);
        if(ret>=0)
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,seperateCmd[4],END);
        }
        else
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,SETERROR,END);
        }
    }
    
    else if(!strcmp(seperateCmd[2],FLOWERWATER)|| !strcmp(seperateCmd[2],PETFOOD))
    {
        int ret = IRCtl(seperateCmd[2],seperateCmd[4]);
        if(ret>=0)
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,seperateCmd[4],END);
        }
        else
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,SETERROR,END);
        }
    }
    else if(!strcmp(seperateCmd[2],STATUS))
    {
        if(!strcmp(seperateCmd[5],PULSE))
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,seperateCmd[4],PULSE,END);
            connectDisconnectTimes=0;
        }
        else
        {
            sprintf(backCmd,"%s:%s:%s:%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK,seperateCmd[4],STATUSGOOD,END);
        }
    }

    else if(!strcmp(seperateCmd[2],ERRORSTATUS))
    {
        printf("%s",backCmd);
        return 0;
    }

    // has bugs in here, feedback standard cmd
    else // can't find cmd, just back it
    {
        sprintf(backCmd,"%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK);
        for(i=4;i<cmdNum;i++)
        {
            strcat(backCmd,":");
            strcat(backCmd,seperateCmd[i]);
        }
    }
    return 1;
    
    
}



int main()
{
    char buf[BUFFER_SIZE];
    int sock_fd;
    struct sockaddr_in servaddr;
    int ret=0;
    
    while(1)
    {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(HOSTPORT);
        inet_pton(AF_INET, SERVERIP, &servaddr.sin_addr);

        if (connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("[client]Can Not Connect To %s, wait 5 minute to retry!\n",SERVERIP);
            sleep(5*60);
            continue;
            //return -1;
        }
        printf("[client]Connect success\n");

        ret = handle(sock_fd);     /* do it all */
        if(ret<0)
        {
            printf("[client]fatal error, wait 5 min to retry!\n");
            sleep(5*60);
        }
        
        close(sock_fd);
    }
    printf("exit\n");
    return 0;
}


int handle(int sock_fd)
{
    char back_buffer[BUFFER_SIZE], rcv_buffer[BUFFER_SIZE];
    fd_set fdsr;
    
    int maxfds = sock_fd+ 1;
    int nread;
	int ret;
	struct timeval tv;
    int times=200;
    connectDisconnectTimes=0;
    
    while(times--) {
	
		tv.tv_sec = TIMEOUT_SEC;
		tv.tv_usec = 0;
		
        FD_ZERO(&fdsr);
        FD_SET(sock_fd, &fdsr);

        ret = select(maxfds, &fdsr, NULL, NULL, &tv);
		if (ret < 0)
		{
			printf("[client]select error\n");
            return -1;
		}
		else if (ret == 0)
		{
            connectDisconnectTimes++;
            if(connectDisconnectTimes>6)
            {
                //network bad, re-connect
                printf("[client]network bad, re-connect\n");
                return 0;
            }
			printf("[client]timeout, loopback\n");
			continue;
		}

        if (FD_ISSET(sock_fd, &fdsr)) {
            //received server responce
            nread = recv(sock_fd, rcv_buffer, BUFFER_SIZE,0);
            if (nread == 0) {
                printf("[client]server close the connection\n");
                return 0;
            } 
            else if (nread < 0) {
                perror("[client]read error");
                return 0;
            }
            else {
                //server response
                int ret;
                printf("rcvcmd is %s\n",rcv_buffer);
                bzero(back_buffer,BUFFER_SIZE);
                ret = distributeHomeService(rcv_buffer,back_buffer);
                
                if(ret){
                    printf("feedback cmd %s\n", back_buffer);
                    if(send(sock_fd,back_buffer,strlen(back_buffer),0)<strlen(back_buffer))
                    {
                        printf("send error\n");
                    }
                }
               
                bzero(rcv_buffer,BUFFER_SIZE);
                

            }  
        }

    }
    
    return 1;
}