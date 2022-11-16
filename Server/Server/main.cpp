#include "stdafx.h"
#include "../../protocol.h"

//DWORD WINAPI ProcessIO(LPVOID _arg)
//{
//
//}

void AcceptClient()
{

	WSADATA wsa;
	SOCKET sock;
	int retval{};

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET)
		err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);		
	retval = bind(listenSock, (struct sockaddr*) & serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	struct sockaddr_in clientAddr;
	int addrlen = 0;
	addrlen = sizeof(clientAddr);

	while (1)
	{
		sock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);
		clients.insert({ cid ,SESSION(cid,sock) });
	}
}

bool SendAddPlayer();

int main()
{
	InitializeCriticalSection(&clientsCS);	// �Ӱ迵�� �ʱ�ȭ

	AcceptClient();

	DeleteCriticalSection(&clientsCS);	// �Ӱ迵�� ����
}

bool SendAddPlayer() {
	SC_ADD_PLAYER packet;
	packet.client_id = cid;	//�ű� Ŭ���̾�Ʈ�� ���̵� ����
	EnterCriticalSection(&clientsCS);
	packet.worldTransform = clients[cid].GetTransform();	// �ű� Ŭ���̾�Ʈ�� ��ġ ����
	for (auto&[id, session] : clients) {
		int result = send(session.GetSocket(), (char*)&packet, sizeof(packet), 0);
		if (result == SOCKET_ERROR) {
			err_display("SendAddPlayer()");
			LeaveCriticalSection(&clientsCS);
			return false;
		}
	}
	LeaveCriticalSection(&clientsCS);
	++cid;
	return true;
}

bool SendWorldData()
{
	SC_WORLD_DATA packet;
	int	retval{};

	packet.my_client_id = cid;		// ������ Ŭ���̾�Ʈ���� id �ο�
	for (auto& [id, session] : clients) {
		++packet.player_count;		// �����ڼ� ī��Ʈ
		if (id == cid)		
			break;
		retval = send(clients[id].GetSocket(), (char*)&packet, sizeof(packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("SendWorldData()");
			return false;
		}
	}

	if (!SendAddPlayer())
		return false;

	return true;

}
