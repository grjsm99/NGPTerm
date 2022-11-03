#include <DirectXMath.h>
#include <minwindef.h>

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
	UCHAR player_count;  // ���� ���� ���� �÷��̾� ��
	USHORT my_client_id; // �ڽ��� �ο����� Ŭ���̾�Ʈ id
	UCHAR missile_count; // ���� ���� ���� �̻����� ����
	// ���� player_count�� ��ŭ SC_ADD_PLAYER�� ������,
	// missile_count�� ��ŭ SC_ADD_MISSILE�� ������.
};
struct SC_ADD_PLAYER { // type = 1
	char type = 1;
	USHORT client_id;	// �߰��� �÷��̾��� id
	XMFLOAT4X4 worldTransform;  // �÷��̾��� ���� ���
};

struct SC_ADD_MISSILE { // type = 2
	char type = 2;
	UINT missile_id;    // �߰��� �̻����� id
	XMFLOAT3 position;			// �߰��� �̻����� ��ġ
	USHORT client_id;	// �̻����� �߻��� Ŭ���̾�Ʈ�� id
	XMFLOAT3 direction;			// �̻����� ����
};

struct SC_MOVE_PLAYER { // type = 3
	char type = 3;
	USHORT client_id;	// �̵��� �÷��̾��� id
	XMFLOAT4X4 worldTransform;  // �÷��̾��� �̵� �� ���� ���
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


// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[����] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
