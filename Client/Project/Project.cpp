﻿// Project.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Project.h"
#include "GameFramework.h"
#include "../../protocol.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void ConnectToServer()
{
    WSADATA wsa;
    int retval{};

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) err_quit("socket()");
    DWORD optval = 1;
    setsockopt(serverSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval));
   
    // connect()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(serverSock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PROJECT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            GameFramework::Instance().FrameAdvance();
            if (GameFramework::Instance().IsGameOver()) {
                break;
            }
        }
    }
    DeleteCriticalSection(&missileCS);
    DeleteCriticalSection(&playerCS);
    GameFramework::Instance().SendPlayerRemove();
    GameFramework::Destroy();

    return (int) msg.wParam;
}

DWORD WINAPI ProcessRecv(LPVOID _curScene)
{
    GameFramework gameFramework = GameFramework::Instance();

    int retval;
    char packetType;
    char* buffer = new char[128];
    size_t packSize[] = { 0, sizeof(SC_ADD_PLAYER) - 1, sizeof(SC_ADD_MISSILE) - 1, sizeof(SC_MOVE_PLAYER) - 1, sizeof(SC_REMOVE_MISSILE) - 1, sizeof(SC_REMOVE_PLAYER) - 1 };
    
    gameFramework.RecvWorldData();
    while (true)
    {
        // 1바이트를 받아 패킷 타입 알아내기
        retval = recv(serverSock, buffer, 1, MSG_WAITALL);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            GameFramework::Instance().SetGameOver();
            return retval;
        }

        packetType = buffer[0];
        // 해당 패킷의 크기만큼 recv
        if (packSize[packetType] > 0) {
            retval = recv(serverSock, buffer + 1, packSize[packetType], MSG_WAITALL);
            if (retval == SOCKET_ERROR) {
                err_display("recv()");
                return retval;
            }
        }

        if (packetType == 1) {
            SC_ADD_PLAYER* packet = reinterpret_cast<SC_ADD_PLAYER*>(buffer);
            gameFramework.AddEnemy(*packet, true);
        }
        if (packetType == 2) {
            SC_ADD_MISSILE* packet = reinterpret_cast<SC_ADD_MISSILE*>(buffer);
            gameFramework.AddMissile(*packet);
        }
        if (packetType == 3) {
            SC_MOVE_PLAYER* packet = reinterpret_cast<SC_MOVE_PLAYER*>(buffer); 
            gameFramework.EnemyMove(*packet);
        }
        if (packetType == 4) {
            SC_REMOVE_MISSILE* packet = reinterpret_cast<SC_REMOVE_MISSILE*>(buffer);
            gameFramework.RemoveMissile(*packet);
        }
        if (packetType == 5) {
            SC_REMOVE_PLAYER* packet = reinterpret_cast<SC_REMOVE_PLAYER*>(buffer);
            gameFramework.RemoveEnemy(*packet);
        }
    }

    return 0;
}

//  함수: MyRegisterClass()
//  용도: 창 클래스를 등록합니다.
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석: 이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//         주 프로그램 창을 만든 다음 표시합니다.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU |
       WS_BORDER;

   RECT windowSize = { 0, 0, 1920, 1080 };
   AdjustWindowRect(&windowSize, dwStyle, FALSE);

   HWND hWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT,
       CW_USEDEFAULT, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, NULL, NULL, hInstance,
       NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ConnectToServer();

   InitializeCriticalSection(&missileCS);
   InitializeCriticalSection(&playerCS);
   GameFramework::Create(hInst, hWnd);
   
   // Recv 쓰레드 생성
   CreateThread(NULL, 0, ProcessRecv, GameFramework::Instance().GetCurrentScene().get(), 0, NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

