#include "stdafx.h"

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
// 소켓 함수 오류 출력
void err_display(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
void err_display(int errcode) {
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


unordered_map<USHORT, SESSION> clients;
int cid, mid;
CRITICAL_SECTION clientsCS;

SESSION::SESSION(USHORT _id, SOCKET& _socket) {
	id = _id;
	socket = _socket;
	transform = Matrix4x4::Identity();
}

SESSION::~SESSION() {

}

const XMFLOAT4X4& SESSION::GetTransform() const {
	return transform;
}

void SESSION::SetTransform(const XMFLOAT4X4& _transform) {
	transform = _transform;
}

const SOCKET& SESSION::GetSocket() const {
	return socket;
}
