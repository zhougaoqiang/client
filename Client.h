#ifndef Client_H_
#define Client_H_
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <conio.h>
#include <iostream>
#include <thread>
#include <WinSock2.h>
#include "sqlite3.h"
#include <stdio.h>
#include <WS2tcpip.h>  //������socklen_t

#pragma comment(lib, "WS2_32")   //���ӵ�WS2_32.lib
#pragma warning(diable:4996);

using namespace std;

#define Server_IP "192.168.1.140"   //��ΪĬ�Ϸ�������IP��ַ
#define Server_Port 8888   //����˿ں�

class client
{
public:
	client();
	void init();
	void process();
	void login();

private:
	int user;
	char username[100];
	char password[100];
	int writing;
	sockaddr_in serverAddress;  //IPv4�ĵ�ַ��������˵�ַ�壬 �����IP��ַ��������˶˿ںš�
	void sendata();
};

client::client()
{
	user = 0;
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

	user = socket(AF_INET, SOCK_STREAM, 0);  //����IPv4��TCP����
	if (user <= 0)
	{
		perror("�����ͻ���ʧ��");
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		exit(1);
	}
	cout << "�����ͻ��˳ɹ�" << endl;

	//����ʽ�ĵȴ�����������
	if (connect(user, (const sockaddr *)&serverAddress, sizeof(serverAddress))< 0)
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
	login();

	while (1)
	{
		FD_SET(user, &fdread);
		if (writing == 0)
			FD_SET(user, &fedwrite);

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
			if (FD_ISSET(user, &fdread)) //���ж��¼�
			{
				int size = recv(user, recvbuff, sizeof(recvbuff) - 1, 0);
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
			if (FD_ISSET(user, &fedwrite))
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
	send(user, sendbuf, sizeof(sendbuf) - 1, 0);
	writing = 0;
}

void client::login()
{
	char passwordvalidation[100];
	char accountCreateStatus[100];
	char pass[7] = "passed";
	memset(accountCreateStatus, '\0', sizeof(accountCreateStatus));
	memset(passwordvalidation, '\0', sizeof(passwordvalidation));


	cout << "1: New user; 2: login\n";
	int accountCreation;
	cin >> accountCreation;
	
	switch (accountCreation)
	{
	case 1:
		char temp2[100];
		while (1)
		{
				char temp1[100] = "New account creation";
				cout << "New account creating...\n";
				send(user, temp1, strlen(temp1), 0);
				cout << "Please enter new username: \n";
				cin >> username;
				send(user, username, strlen(username), 0);
				cout << "Please enter your password: \n";
				cin >> password;
				send(user, password, strlen(password), 0);


				int size = recv(user, accountCreateStatus, sizeof(accountCreateStatus), 0);
				if (size > 0)
				{
					char temprec[] = "User account creation successfully";
					if (strcmp(temprec, accountCreateStatus) == 0)
					{
						cout << accountCreateStatus << endl;
						break;
					}
					else
					{
						cout << accountCreateStatus << endl;
						exit(1);
					}
				}
				else
				{
					cout << "connection failed." << endl;
					exit(1);
				}
				break;
		}
		break;
	default:
		while (1)
		{
			char temp2[100] = "Exist account";
			send(user, temp2, strlen(temp2), 0);
			cout << "Please enter username: ";
			cin >> username;
			send(user, username, strlen(username), 0);

			cout << "Plase enter password: ";
			cin >> password;
			send(user, password, strlen(password), 0);

			int size = recv(user, passwordvalidation, sizeof(passwordvalidation), 0);
			if (size > 0)
			{
				if (strcmp(pass, passwordvalidation) == 0)
				{
					cout << "login success\n";
					break;
				}
				else
				{
					cout << "login falied\n";
					exit(1);
				}
			}
			else
			{
				cout << "connection failed." << endl;
				exit(1);
			}
		}
	}
	
	
}
#endif // !Client_H_

