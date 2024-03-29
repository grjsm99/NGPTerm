#pragma once

#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

#include <iostream>

#include <DirectXMath.h>
#include <minwindef.h>
#include <unordered_map>

#define SERVERPORT 9000
#define BUFSIZE    1000

using namespace DirectX;
using namespace std;

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg);

// 소켓 함수 오류 출력
void err_display(const char* msg);
void err_display(int errcode);

class SESSION {
private:
	USHORT id;
	SOCKET socket;
	XMFLOAT3 localPosition; // 이동한 플레이어의 위치
	XMFLOAT4 localRotation; // 이동한 플레이어의 회전값

public:
	// 생성자 & 소멸자
	SESSION() = default;
	SESSION(USHORT _id, SOCKET& _socket);
	virtual ~SESSION();

	// get, set함수
	const XMFLOAT3& GetLocalPosition() const { return localPosition; };
	void SetLocalPosition(const XMFLOAT3& _localPosition) { localPosition = _localPosition; };
	const XMFLOAT4& GetLocalRotation() const { return localRotation; };
	void SetLocalRotation(const XMFLOAT4& _localRotation) { localRotation = _localRotation; };
	
	USHORT GetID() const { return id; };
	const SOCKET& GetSocket() const;

};

extern unordered_map<USHORT, SESSION> clients;
extern int cid, mid;
extern CRITICAL_SECTION clientsCS;

namespace Matrix4x4 {
	inline XMFLOAT4X4 Identity() {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixIdentity());
		return result;
	}
}

