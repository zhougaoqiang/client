#ifndef Client_H_
#define Client_H_
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <conio.h>
#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <stdio.h>
#include <WS2tcpip.h>  //定义在socklen_t

#pragma comment(lib, "WS2_32")   //链接到WS2_32.lib

using namespace std;

#define Server_IP "127.0.0.1"   //此为默认服务器端IP地址
#define Server_Port 8888   //服务端口号

class client
{
public:
	client();
	void init();
	void process();

private:
	int users;
	int writing;
	sockaddr_in serverAddress;  //IPv4的地址包括服务端地址族， 服务端IP地址，　服务端端口号。
	void sendata();
};

client::client()
{
	users = 0;
	writing = 0;
	serverAddress.sin_family = PF_INET; 
	serverAddress.sin_port = Server_Port;
	serverAddress.sin_addr.s_addr = inet_addr(Server_IP); //将字符串转换为uint32_t
}

void client::init()
{
	int Ret;
	WSADATA wsaData;  //用于初始化套接字环境
					//初始化winsock环境
	if ((Ret = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0)
	{
		cout << "WSAStartup() failed with error %d\n" << Ret;
		WSACleanup();
	}

	users = socket(AF_INET, SOCK_STREAM, 0);  //采用IPv4，TCP传输
	if (users <= 0)
	{
		perror("创建客户端失败");
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		exit(1);
	}
	cout << "创建客户端成功" << endl;

	//阻塞式的等待服务器连接
	if (connect(users, (const sockaddr *)&serverAddress, sizeof(serverAddress))< 0)
	{
		perror("连接服务器失败");
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		exit(1);
	}

	cout << "connect IP: " << Server_IP << "  Port: " << Server_Port << endl; //创建成功
}

void client::process() {
	char recvbuff[1024];
	fd_set fdread, fedwrite;
	FD_ZERO(&fdread); //将fds清零
	FD_ZERO(&fedwrite); //将fds清零

	init();

	while (1)
	{
		FD_SET(users, &fdread);
		if (writing == 0)
			FD_SET(users, &fedwrite);

		struct timeval timeout = { 1, 0 }; //每个select等待3秒

		switch (select(0, &fdread, &fedwrite, NULL, &timeout))
		{
		case -1:
		{
			//perror("select")
			cout << "Error at socket(): " << WSAGetLastError() << endl;
			//exit(1)
			break;
		}
		case 0:
		{
			//cout << "Select timeout......"
			break;
		}
		default:
		{
			if (FD_ISSET(users, &fdread)) //则有读事件
			{
				int size = recv(users, recvbuff, sizeof(recvbuff) - 1, 0);
				if (size > 0)
				{
					cout << "Server: " << recvbuff << endl;
					memset(recvbuff, '\0', sizeof(recvbuff));
				}
				else if (size == 0)
				{
					cout << "Server is closed\n";
					exit(1);
				}
			}
			if (FD_ISSET(users, &fedwrite))
			{
				FD_ZERO(&fedwrite); //将fedwrite清零
				writing = 1; //表示正在写作
				thread sendtask(bind(&client::sendata, this));
				sendtask.detach();  //将子线程和主进程分离互相不影响
			}

			break;
		}
		}

	}
}

void client::sendata()
{
	char sendbuf[1024];
	char middle[1024];

	cin.getline(sendbuf, 1024); //读取一行
	send(users, sendbuf, sizeof(sendbuf) - 1, 0);
	writing = 0;
}
#endif // !Client_H_

