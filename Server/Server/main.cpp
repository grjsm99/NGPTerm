#include "stdafx.h"
#include "../../protocol.h"


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
	while(1)
	{
		sock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);
		clients.insert({ cid ,SESSION(cid,sock) });
		++cid;
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
	SC_WORLD_DATA WorldDataPacket;
	SC_ADD_PLAYER AddPlayerPacket;
	int	retval{};

	for (auto& [id, session] : clients) {			// �÷��̾� �� ��ŭ ī��Ʈ���ְ� ������ �ִ� �÷��̾�� ��Ŷ�� ����ֱ�
		++WorldDataPacket.player_count;
		AddPlayerPacket.client_id = id;
		AddPlayerPacket.worldTransform = session.GetTransform();
	}
	
	retval = send(clients[cid].GetSocket(), (char*)&WorldDataPacket, sizeof(WorldDataPacket), 0);	// ���� ���� ���� ����
	if (retval == SOCKET_ERROR) {
		err_display("SendWorldData()");
		return false;
	}

	for (int i = 1; i != WorldDataPacket.player_count; i++)
	{
		retval = send(clients[cid].GetSocket(), (char*)&AddPlayerPacket, sizeof(AddPlayerPacket), 0);	// ���� Ŭ���̾�Ʈ ������ ����
		if (retval == SOCKET_ERROR) {
			err_display("SendExistingClientsData()");
			return false;
		}
	}


	return true;

}