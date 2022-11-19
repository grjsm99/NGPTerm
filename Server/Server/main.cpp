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
	InitializeCriticalSection(&clientsCS);	// 임계영역 초기화

	AcceptClient();

	DeleteCriticalSection(&clientsCS);	// 임계영역 삭제
}

// 특정 플레이어가 미사일을 발사 시 모든 플레이어에게 이사실을 전송한다.
bool SendAddMissile(USHORT _cid) {
	SC_ADD_MISSILE packet;
	EnterCriticalSection(&clientsCS);

	packet.missile_id = mid++;		// 0번부터 시작
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


// 신규 유저에 대한 정보를 패킷에 담아 미리 접속해 있던 모든 플레이어에게 전송한다.
bool SendAddPlayer(const SESSION& _Session) {
	SC_ADD_PLAYER packet;
	packet.client_id = _Session.GetID();	//신규 클라이언트의 아이디 적재

	packet.localPosition = _Session.GetLocalPosition();	// 신규 클라이언트의 위치 적재
	packet.localRotation = _Session.GetLocalRotation();	// 신규 클라이언트의 위치 적재
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

// 신규 유저에게 미리 접속해 있던 플레이어들의 정보를 전부 보낸다.
bool SendWorldData(const SESSION& _Session)
{
	SC_WORLD_DATA WorldDataPacket;
	vector<SC_ADD_PLAYER> addPlayer;
	int	retval{};
	WorldDataPacket.player_count = clients.size();

	retval = send(_Session.GetSocket(), (char*)&WorldDataPacket, sizeof(WorldDataPacket), 0);	// 현재 월드 정보 전송
	if (retval == SOCKET_ERROR) {
		err_display("SendWorldData()");
		return false;
	}

	for (auto& [id, session] : clients) {			// 플레이어 수 만큼 카운트해주고 접속해 있는 플레이어들 패킷에 담아주기
		SC_ADD_PLAYER AddPlayerPacket;
		AddPlayerPacket.client_id = id;
		AddPlayerPacket.localPosition = session.GetLocalPosition();
		AddPlayerPacket.localRotation = session.GetLocalRotation();
		addPlayer.push_back(AddPlayerPacket);
	}
	retval = send(_Session.GetSocket(), (char*)addPlayer.data(), sizeof(SC_ADD_PLAYER)* addPlayer.size(), 0);	// 기존 클라이언트 정보들 전송
	if (retval == SOCKET_ERROR) {
		err_display("SendExistingClientsData()");
		return false;
	}
	
	return true;

}