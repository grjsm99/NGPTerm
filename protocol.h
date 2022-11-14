#pragma once

#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

#include <DirectXMath.h>
#include <minwindef.h>

#include <stdio.h>
#include <iostream>

using namespace DirectX;

#pragma pack(push, 1)

struct CS_MOVE_PLAYER { // type = 0
	XMFLOAT4X4 worldTransform; // 플레이어의 이동 후 월드 행렬
};

struct CS_ADD_MISSILE { // type = 1
	char type = 1;
};

struct CS_REMOVE_MISSILE { // type = 2
	char type = 2;
	UINT missile_id;  // 삭제할 미사일의 id
};

struct CS_REMOVE_PLAYER { // type = 3
	char type = 3;
};


/////////////////////////////////////////////////////////


struct SC_WORLD_DATA { // type = 0
	char type = 0;
	UCHAR player_count;  // 현재 월드 내의 플레이어 수
	USHORT my_client_id; // 자신이 부여받을 클라이언트 id

	// 이후 player_count값 만큼 SC_ADD_PLAYER를 보낸다.
};

struct SC_ADD_PLAYER { // type = 1
	char type = 1;
	USHORT client_id;	// 추가할 플레이어의 id
	XMFLOAT4X4 worldTransform;  // 플레이어의 월드 행렬
};

struct SC_ADD_MISSILE { // type = 2
	char type = 2;
	UINT missile_id;    // 추가할 미사일의 id
	XMFLOAT3 position;		// 추가할 미사일의 위치
	USHORT client_id;	// 미사일을 발사한 클라이언트의 id
	XMFLOAT4 rotation;  // 미사일의 회전값
};

struct SC_MOVE_PLAYER { // type = 3
	char type = 3;
	USHORT client_id;	// 이동한 플레이어의 id
	XMFLOAT3 localPosition; // 이동한 플레이어의 위치
	XMFLOAT4 localRotation; // 이동한 플레이어의 회전값
};

struct SC_REMOVE_MISSILE { // type = 4
	char type = 4;
	UINT missile_id;	// 삭제할 미사일의 id
};

struct SC_REMOVE_PLAYER { // id = 5
	char type = 5;
	USHORT client_id;	// 삭제할 플레이어의 id
};

#pragma pack(pop)
