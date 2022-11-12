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


class SESSION {
private:
	USHORT id;
	SOCKET socket;
	XMFLOAT4X4 transform;

public:
	// 생성자 & 소멸자
	SESSION(USHORT _id, SOCKET& _socket);
	virtual ~SESSION();

	// get, set함수
	const XMFLOAT4X4& GetTransform() const;
	void SetTransform(const XMFLOAT4X4& _transform);

};

extern unordered_map<USHORT, SESSION> clients;
extern USHORT cid = 0, mid = 0;

namespace Matrix4x4 {
	inline XMFLOAT4X4 Identity() {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixIdentity());
		return result;
	}
}
