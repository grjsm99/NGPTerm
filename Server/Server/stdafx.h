#pragma once

#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���
#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

#include <iostream>

#include <DirectXMath.h>
#include <minwindef.h>
#include <unordered_map>

#define SERVERPORT 9000
#define BUFSIZE    1000

using namespace DirectX;
using namespace std;

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg);

// ���� �Լ� ���� ���
void err_display(const char* msg);
void err_display(int errcode);

class SESSION {
private:
	USHORT id;
	SOCKET socket;
	XMFLOAT4X4 transform;

public:
	// ������ & �Ҹ���
	SESSION() = default;
	SESSION(USHORT _id, SOCKET& _socket);
	virtual ~SESSION();

	// get, set�Լ�
	const XMFLOAT4X4& GetTransform() const;
	void SetTransform(const XMFLOAT4X4& _transform);
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

