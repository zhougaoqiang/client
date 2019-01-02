#ifndef Client_H_
#define Client_H_
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <conio.h>
#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <stdio.h>
#include <WS2tcpip.h>  //������socklen_t

#pragma comment(lib, "WS2_32")   //���ӵ�WS2_32.lib

using namespace std;

#define Server_IP "127.0.0.1"   //��ΪĬ�Ϸ�������IP��ַ
#define Server_Port 8888   //����˿ں�

class client
{
public:
	client();
	void init();
	void process();

private:
	int users;
	int writing;
	sockaddr_in serverAddress;  //IPv4�ĵ�ַ��������˵�ַ�壬 �����IP��ַ��������˶˿ںš�
	void sendata();
};

client::client()
{
	users = 0;
	writing = 0;
	serverAddress.sin_family = PF_INET; 
	serverAddress.sin_port = Server_Port;
	serverAddress.sin_addr.s_addr = inet_addr(Server_IP); //���ַ���ת��Ϊuint32_t
}

void client::init()
{
	int Ret;
	WSADATA wsaData;  //���ڳ�ʼ���׽��ֻ���
					//��ʼ��winsock����
	if ((Ret = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0)
	{
		cout << "WSAStartup() failed with error %d\n" << Ret;
		WSACleanup();
	}

	users = socket(AF_INET, SOCK_STREAM, 0);  //����IPv4��TCP����
	if (users <= 0)
	{
		perror("�����ͻ���ʧ��");
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		exit(1);
	}
	cout << "�����ͻ��˳ɹ�" << endl;

	//����ʽ�ĵȴ�����������
	if (connect(users, (const sockaddr *)&serverAddress, sizeof(serverAddress))< 0)
	{
		perror("���ӷ�����ʧ��");
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		exit(1);
	}

	cout << "connect IP: " << Server_IP << "  Port: " << Server_Port << endl; //�����ɹ�
}

void client::process() {
	char recvbuff[1024];
	fd_set fdread, fedwrite;
	FD_ZERO(&fdread); //��fds����
	FD_ZERO(&fedwrite); //��fds����

	init();

	while (1)
	{
		FD_SET(users, &fdread);
		if (writing == 0)
			FD_SET(users, &fedwrite);

		struct timeval timeout = { 1, 0 }; //ÿ��select�ȴ�3��

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
			if (FD_ISSET(users, &fdread)) //���ж��¼�
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
				FD_ZERO(&fedwrite); //��fedwrite����
				writing = 1; //��ʾ����д��
				thread sendtask(bind(&client::sendata, this));
				sendtask.detach();  //�����̺߳������̷��뻥�಻Ӱ��
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

	cin.getline(sendbuf, 1024); //��ȡһ��
	send(users, sendbuf, sizeof(sendbuf) - 1, 0);
	writing = 0;
}
#endif // !Client_H_

