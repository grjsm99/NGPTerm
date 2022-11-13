#include "stdafx.h"
#include "../../protocol.h"

SOCKET sock;

void AcceptClient()
{
	WSADATA wsa;
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

	sock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);
	clients.insert({ cid ,SESSION(cid,sock) });
	++cid;

}

bool SendAddPlayer();

int main()
{
	InitializeCriticalSection(&clientsCS);	// 임계영역 초기화

	AcceptClient();

	DeleteCriticalSection(&clientsCS);	// 임계영역 삭제
}

bool SendAddPlayer() {
	SC_ADD_PLAYER packet;
	packet.client_id = cid;	//신규 클라이언트의 아이디 적재
	EnterCriticalSection(&clientsCS);
	packet.worldTransform = clients[cid].GetTransform();	// 신큐 클라이언트의 위치 적재
	for (auto&[id, session] : clients) {
		int result = send(session.GetSocket(), (char*)&packet, sizeof(packet), 0);
		if (result == SOCKET_ERROR) {
			err_display("SendAddPlayer()");
			return false;
		}
	}
	LeaveCriticalSection(&clientsCS);
	++cid;
	return true;
}
