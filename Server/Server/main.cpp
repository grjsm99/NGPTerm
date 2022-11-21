#include "stdafx.h"
#include "../../protocol.h"

void AcceptClient();
bool SendAddMissile(USHORT _cid);
bool SendAddPlayer(const SESSION& _Session);
bool SendWorldData(const SESSION& _Session);
DWORD WINAPI ProcessIO(LPVOID _arg);



int main()
{

	HANDLE						hThread;
	while (1)
	{
		InitializeCriticalSection(&clientsCS);	// �Ӱ迵�� �ʱ�ȭ

		AcceptClient();

		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessIO, (LPVOID)clients[cid].GetID(), 0, NULL);
		if (hThread == NULL) {
			cout << cid << "socket is NULL" << endl;
			closesocket(clients[cid].GetSocket());
		}
		else { CloseHandle(hThread); }

		DeleteCriticalSection(&clientsCS);	// �Ӱ迵�� ����
	}

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

DWORD WINAPI ProcessIO(LPVOID _arg)
{
	int				 retval{};
	char			 buffer[256];
	char			 packetType{};
	int				 packetSize[4] = { sizeof(CS_MOVE_PLAYER), sizeof(CS_ADD_MISSILE), sizeof(CS_REMOVE_MISSILE), sizeof(CS_REMOVE_PLAYER) };
	char			 type{};
	SOCKET			 client_sock = clients[(USHORT)_arg].GetSocket();
	sockaddr_in		 clientAddr;
	char			 addr[INET_ADDRSTRLEN];
	int				 addrLen{};

	// Ŭ���̾�Ʈ ���� ���
	addrLen = sizeof(clientAddr);
	getpeername(client_sock, (struct sockaddr*)&clientAddr, &addrLen);
	inet_ntop(AF_INET, &clientAddr.sin_addr, addr, sizeof(addr));

	while (1)
	{
		// ��ŶŸ�� recv()
		retval = recv(client_sock, (char*)&type, 1, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			return 0;
		}

		packetType = type;

		// ���� ��Ŷ�� �����ŭ recv()
		retval = recv(client_sock, buffer, packetSize[packetType], 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recv()");
			return 0;
		}

		// ��Ŷ Ÿ�Կ� ���� �޼��� ó���ϵ��� ����
		switch (packetType)
		{
		case 0:		// �÷��̾� �̵� ���� ó��
			//  SendMovePlayer();


		case 1:		// �̻��� �߰� ���� ó��
			cout << "recv()_1" << endl;
			if (!SendAddMissile((USHORT)_arg))
				return 0;
			break;

		case 2:		// �̻��� ���� ���� ó��
			// SendRemoveMissile()
			break;

		case 3:		// �÷��̾� ���� ���� ó��
			// SendRemovePlayer()
			break;

		default:
			cout << "�߸��� ��ŶŸ��" << endl;
		}

	}
}
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
	retval = bind(listenSock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");


	struct sockaddr_in clientAddr;
	int addrlen = 0;
	addrlen = sizeof(clientAddr);


		sock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);
		SESSION newSession(cid, sock);
		SendWorldData(newSession);
		SendAddPlayer(newSession);
		clients.insert({ cid , newSession });
		cout << "Accept client[" << cid << "]" << endl;
		++cid;


}