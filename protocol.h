#include <DirectXMath.h>
#include <minwindef.h>

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

class SESSION {
private:
	USHORT id;
	XMFLOAT4X4 worldTransform;
};

class MISSILE {
private:
	UINT id;
	XMFLOAT3 position;
	XMFLOAT3 direction;
};


struct SC_WORLD_DATA { // type = 0
	char type = 0;
	UCHAR player_count;  // 현재 월드 내의 플레이어 수
	USHORT my_client_id; // 자신이 부여받을 클라이언트 id
	UCHAR missile_count; // 현재 월드 내의 미사일의 개수
	// 이후 player_count값 만큼 SC_ADD_PLAYER를 보내고,
	// missile_count값 만큼 SC_ADD_MISSILE을 보낸다.
};
struct SC_ADD_PLAYER { // type = 1
	char type = 1;
	USHORT client_id;	// 추가할 플레이어의 id
	XMFLOAT4X4 worldTransform;  // 플레이어의 월드 행렬
};

struct SC_ADD_MISSILE { // type = 2
	char type = 2;
	UINT missile_id;    // 추가할 미사일의 id
	XMFLOAT3 position;			// 추가할 미사일의 위치
	USHORT client_id;	// 미사일을 발사한 클라이언트의 id
	XMFLOAT3 direction;			// 미사일의 방향
};

struct SC_MOVE_PLAYER { // type = 3
	char type = 3;
	USHORT client_id;	// 이동한 플레이어의 id
	XMFLOAT4X4 worldTransform;  // 플레이어의 이동 후 월드 행렬
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


// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
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
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
