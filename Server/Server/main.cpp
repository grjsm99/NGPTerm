#include "stdafx.h"
#include "../../protocol.h"
bool SendAddMissile(USHORT _cid);
bool SendAddPlayer(const SESSION& _Session);
bool SendWorldData(const SESSION& _Session);

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
		SESSION newSession(cid, sock);
		SendWorldData(newSession);
		SendAddPlayer(newSession);
		clients.insert({ cid , newSession });
		++cid;

	}

}

int main()
{
	InitializeCriticalSection(&clientsCS);	// �Ӱ迵�� �ʱ�ȭ

	AcceptClient();

	DeleteCriticalSection(&clientsCS);	// �Ӱ迵�� ����
}

// Ư�� �÷��̾ �̻����� �߻� �� ��� �÷��̾�� �̻���� �����Ѵ�.
bool SendAddMissile(USHORT _cid) {
	SC_ADD_MISSILE packet;
	EnterCriticalSection(&clientsCS);

	packet.missile_id = mid++;		// 0������ ����
	packet.client_id = clients[_cid].GetID();
	packet.position = clients[_cid].GetLocalPosition();
	packet.rotation = clients[_cid].GetLocalRotation();
	for (auto& [id, session] : clients) {
		int result = send(session.GetSocket(), (char*)&packet, sizeof(packet), 0);
		if (result == SOCKET_ERROR) {
			err_display("SendAddPlayer()");
			LeaveCriticalSection(&clientsCS);
			return false;
		}
	}
	LeaveCriticalSection(&clientsCS);
	return true;
}


// �ű� ������ ���� ������ ��Ŷ�� ��� �̸� ������ �ִ� ��� �÷��̾�� �����Ѵ�.
bool SendAddPlayer(const SESSION& _Session) {
	SC_ADD_PLAYER packet;
	packet.client_id = _Session.GetID();	//�ű� Ŭ���̾�Ʈ�� ���̵� ����

	packet.localPosition = _Session.GetLocalPosition();	// �ű� Ŭ���̾�Ʈ�� ��ġ ����
	packet.localRotation = _Session.GetLocalRotation();	// �ű� Ŭ���̾�Ʈ�� ��ġ ����
	EnterCriticalSection(&clientsCS);
	for (auto&[id, session] : clients) {
		int result = send(session.GetSocket(), (char*)&packet, sizeof(packet), 0);
		if (result == SOCKET_ERROR) {
			err_display("SendAddPlayer()");
			LeaveCriticalSection(&clientsCS);
			return false;
		}
	}
	LeaveCriticalSection(&clientsCS);
	return true;
}

// �ű� �������� �̸� ������ �ִ� �÷��̾���� ������ ���� ������.
bool SendWorldData(const SESSION& _Session)
{
	SC_WORLD_DATA WorldDataPacket;
	vector<SC_ADD_PLAYER> addPlayer;
	int	retval{};
	WorldDataPacket.player_count = clients.size();

	retval = send(_Session.GetSocket(), (char*)&WorldDataPacket, sizeof(WorldDataPacket), 0);	// ���� ���� ���� ����
	if (retval == SOCKET_ERROR) {
		err_display("SendWorldData()");
		return false;
	}

	for (auto& [id, session] : clients) {			// �÷��̾� �� ��ŭ ī��Ʈ���ְ� ������ �ִ� �÷��̾�� ��Ŷ�� ����ֱ�
		SC_ADD_PLAYER AddPlayerPacket;
		AddPlayerPacket.client_id = id;
		AddPlayerPacket.localPosition = session.GetLocalPosition();
		AddPlayerPacket.localRotation = session.GetLocalRotation();
		addPlayer.push_back(AddPlayerPacket);
	}
	retval = send(_Session.GetSocket(), (char*)addPlayer.data(), sizeof(SC_ADD_PLAYER)* addPlayer.size(), 0);	// ���� Ŭ���̾�Ʈ ������ ����
	if (retval == SOCKET_ERROR) {
		err_display("SendExistingClientsData()");
		return false;
	}
	
	return true;

}