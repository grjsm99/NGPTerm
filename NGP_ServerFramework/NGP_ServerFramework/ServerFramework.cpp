#include "protocol.h"

DWORD WINAPI Accept(LPVOID arg)
{
	WSADATA wsa;
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	SOCKET litenSock = socket(AF_INET, SOCK_STREAM, 0);

	// 오류 검출
	if (litenSock == INVALID_SOCKET)
	{
		int errCode = WSAGetLastError();
		cout << "Socket ErrorCode " << endl;
		return;
	}

	// local addr
	SOCKADDR_IN serverAddr;		
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);		// INADDR_ANY: 니가 알아서 골라줘
	serverAddr.sin_port = htons(7777);

	// bind(): 소켓의 지역 IP 주소와 지역 포트 번호를 결정
	if (bind(litenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		cout << "Bind ErrorCode " << endl;
		return;
	}

	// listen()
	if (listen(litenSock, 3) == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		cout << "Listen ErrorCode " << endl;
		return;
	}

	SOCKADDR_IN clientAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	int addrLen = sizeof(clientAddr);

	SOCKET clientSocket = accept(litenSock, (SOCKADDR*)&clientAddr, &addrLen);

}


int main()
{


	AcceptToClient();
}