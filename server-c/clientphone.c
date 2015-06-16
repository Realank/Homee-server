#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <pthread.h>

#define HELLO_WORLD_SERVER_PORT    6001 
#define BUFFER_SIZE 100

void *run()
{

	char* ip = "127.0.0.1";
    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET;    //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //把客户机的socket和客户机的socket地址结构联系起来
    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(ip,&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n",ip);
        exit(1);
    }
	printf("connect ok!\n");

    
    char buffer[BUFFER_SIZE];

    //从服务器接收数据到buffer中
	if(send(client_socket,"LITE:SETE:LVRM:0001:0002",BUFFER_SIZE,0)<=0)
	{
		printf("send error\n");
	}
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
    if( length = recv(client_socket,buffer,BUFFER_SIZE,0))
    {
        if(length <= 0)
        {
            printf("Recieve Data From Server %s Failed!\n", ip);
            //break;
			goto END;
        }
		int cmdNum=(strlen(buffer)+1)/5;
		char cmd[6][5];
        if(cmdNum>6)
            cmdNum=6;
		int i;
		printf("rcvcmd is %s\n",buffer);
        for(i=0;i<cmdNum;i++)
        {
            strncpy(cmd[i],buffer+i*5,4);
			cmd[i][4]=0;
            //("cmd %d is %s\n",i,cmd[i]);
        }
        bzero(buffer,BUFFER_SIZE);
		sprintf(buffer,"%s:%s:%s",cmd[0],"BKHM",cmd[2]);
		//send(client_socket,buffer,BUFFER_SIZE,0);
        //bzero(buffer,BUFFER_SIZE);    
    }
	else
	{
		printf("length is %d\n",length);
	}
END: printf("Recieve data From Server[%s] Finished\n", ip);
    
    //关闭socket
    close(client_socket);
    return 0;
}

int main ()
{
    pthread_t thread1_id;
    pthread_t thread2_id;
	printf("main\n");
    pthread_create (&thread1_id, NULL, run, NULL);//connect to home
    pthread_create (&thread2_id, NULL, run, NULL);//connect to phone client

    pthread_join (thread1_id, NULL);

    pthread_join (thread2_id, NULL);

    return 0;
}