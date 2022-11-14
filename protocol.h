#pragma once

#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���
#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

#include <DirectXMath.h>
#include <minwindef.h>

#include <stdio.h>
#include <iostream>

using namespace DirectX;

#pragma pack(push, 1)

struct CS_MOVE_PLAYER { // type = 0
	XMFLOAT4X4 worldTransform; // �÷��̾��� �̵� �� ���� ���
};

struct CS_ADD_MISSILE { // type = 1
	char type = 1;
};

struct CS_REMOVE_MISSILE { // type = 2
	char type = 2;
	UINT missile_id;  // ������ �̻����� id
};

struct CS_REMOVE_PLAYER { // type = 3
	char type = 3;
};


/////////////////////////////////////////////////////////


struct SC_WORLD_DATA { // type = 0
	char type = 0;
	UCHAR player_count;  // ���� ���� ���� �÷��̾� ��
	USHORT my_client_id; // �ڽ��� �ο����� Ŭ���̾�Ʈ id

	// ���� player_count�� ��ŭ SC_ADD_PLAYER�� ������.
};

struct SC_ADD_PLAYER { // type = 1
	char type = 1;
	USHORT client_id;	// �߰��� �÷��̾��� id
	XMFLOAT4X4 worldTransform;  // �÷��̾��� ���� ���
};

struct SC_ADD_MISSILE { // type = 2
	char type = 2;
	UINT missile_id;    // �߰��� �̻����� id
	XMFLOAT3 position;		// �߰��� �̻����� ��ġ
	USHORT client_id;	// �̻����� �߻��� Ŭ���̾�Ʈ�� id
	XMFLOAT4 rotation;  // �̻����� ȸ����
};

struct SC_MOVE_PLAYER { // type = 3
	char type = 3;
	USHORT client_id;	// �̵��� �÷��̾��� id
	XMFLOAT3 localPosition; // �̵��� �÷��̾��� ��ġ
	XMFLOAT4 localRotation; // �̵��� �÷��̾��� ȸ����
};

struct SC_REMOVE_MISSILE { // type = 4
	char type = 4;
	UINT missile_id;	// ������ �̻����� id
};

struct SC_REMOVE_PLAYER { // id = 5
	char type = 5;
	USHORT client_id;	// ������ �÷��̾��� id
};

#pragma pack(pop)
