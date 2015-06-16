#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include "server.h"

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
	static livingRmLightStatus=0;
	static bedRm01LightStatus=0;
	static bedRm02LightStatus=0;
	static bedRm03LightStatus=0;
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

void distributeHomeService(char* buf,char* backCmd)
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
	
	else // can't find cmd, just back it
	{
		sprintf(backCmd,"%s:%s:%s:%s",seperateCmd[0],seperateCmd[1],seperateCmd[2],BACK);
		for(i=4;i<cmdNum;i++)
		{
			strcat(backCmd,":");
			strcat(backCmd,seperateCmd[i]);
		}
	}
	  
    
}

int main(int argc, char **argv)
{

	char rcv_buffer[BUFFER_SIZE];
	char back_buffer[BUFFER_SIZE];
    //����һ��socket��ַ�ṹclient_addr,����ͻ���internet��ַ, �˿�
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr)); //��һ���ڴ���������ȫ������Ϊ0
    client_addr.sin_family = AF_INET;    //internetЭ����
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY��ʾ�Զ���ȡ������ַ
    client_addr.sin_port = htons(0);    //0��ʾ��ϵͳ�Զ�����һ�����ж˿�
    //��������internet����Э��(TCP)socket,��client_socket����ͻ���socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //�ѿͻ�����socket�Ϳͻ�����socket��ַ�ṹ��ϵ����
    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }

    //����һ��socket��ַ�ṹserver_addr,�����������internet��ַ, �˿�
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(SERVERIP,&server_addr.sin_addr) == 0) //��������IP��ַ���Գ���Ĳ���
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HOSTPORT);
    socklen_t server_addr_length = sizeof(server_addr);
    //���������������,���ӳɹ���client_socket�����˿ͻ����ͷ�������һ��socket����
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n",SERVERIP);
        exit(1);
    }

    
    

    //�ӷ������������ݵ�buffer��
    bzero(rcv_buffer,BUFFER_SIZE);
    int length = 0;
    while( length = recv(client_socket,rcv_buffer,BUFFER_SIZE,0))
    {
        if(length < 0)
        {
            printf("Recieve Data From Server %s Failed!\n", SERVERIP);
            break;
        }
		printf("rcvcmd is %s\n",rcv_buffer);
		
		distributeHomeService(rcv_buffer,back_buffer);
		if(send(client_socket,back_buffer,BUFFER_SIZE,0)<BUFFER_SIZE)
		{
			printf("send error\n");
		}
        bzero(rcv_buffer,BUFFER_SIZE);    
    }
	printf("Recieve data From Server[%s] Finished\n", SERVERIP);
    
    //�ر�socket
    close(client_socket);
    return 0;
}