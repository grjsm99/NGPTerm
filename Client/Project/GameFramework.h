#pragma once
#include "..\..\protocol.h"
#include "Timer.h"
#include "Scene.h"

class GameFramework {
// 정적 변수 
private:
	static unique_ptr<GameFramework> spInstance;	// 고유 프레임워크 

public:
	static void Create(HINSTANCE _hInstance, HWND _hMainWnd);
	static void Destroy();

	// 프레임워크 인스턴스를 전역으로 접근 하기 위한 함수
	static GameFramework& Instance();
	

private:
	HINSTANCE instanceHandle;
	HWND windowHandle;

	ComPtr<IDXGIFactory4> pDxgiFactory;										// DXGI 팩토리 인터페이스에 대한 포인터
	ComPtr<IDXGISwapChain3> pDxgiSwapChain;									// 스왑체인 인터페이스
	ComPtr<ID3D12Device> pDevice; 

	bool msaa4xEnable;
	UINT msaa4xLevel;

	static const UINT nSwapChainBuffer = 2;									// 스왑 체인 버퍼 개수
	UINT swapChainBufferCurrentIndex;										// 현재 그릴 스왑체인의 후면버퍼 인덱스 

	// 렌더타겟, 깊이-스텐실 버퍼
	array<ComPtr<ID3D12Resource>, nSwapChainBuffer> pRenderTargetBuffers;	// 렌더 타겟 버퍼(후면) 포인터를 담는 배열
	ComPtr<ID3D12DescriptorHeap> pRtvDescriptorHeap;						// 렌더타겟버퍼의 서술자 힙. 
	UINT rtvDescriptorIncrementSize;										// 렌더타겟 서술자의 크기

	ComPtr<ID3D12Resource> pDepthStencilBuffer;								// 깊이-스텐실 버퍼 포인터를 담는 배열
	ComPtr<ID3D12DescriptorHeap> pDsvDescriptorHeap;						// 깊이-스텐실 버퍼의 서술자 힙. 
	UINT dsvDescriptorIncrementSize;										// 깊이-스텐실 버퍼의 서술자의 크기


	// 명령 큐, 명령 할당자, 명령 리스트의 인터페이스 포인터
	ComPtr<ID3D12CommandQueue> pCommandQueue;
	ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> pCommandList;

	// 펜스
	ComPtr<ID3D12Fence> pFence;
	HANDLE fenceEvent;
	array<UINT64, nSwapChainBuffer> fenceValues;
	
	// 루트 시그니처
	ComPtr<ID3D12RootSignature> pRootSignature;

	// 게임 타이머
	Timer gameTimer;


	///// Instance()에서 다른 함수로 접근 가능한 변수들

	bool drawHitBox;
	int clientWidth;
	int clientHeight;
	// 메쉬 메니저
	MeshManager meshManager;
	// 마테리얼 매니저
	TextureManager textureManager;
	// 게임 오브젝트 매니저
	GameObjectManager gameObjectManager;
	// 루트 시그니처 Get
	ComPtr<ID3D12RootSignature> GetRootSignature() { return pRootSignature; }

	// 씬
	stack<shared_ptr<Scene>> pScenes;

	// Run
	bool gameOver;

// 생성, 소멸자
private:
	GameFramework();
public:
	~GameFramework();

// 생성시 초기화용 함수
protected:
	void CreateDirect3DDevice();		// dxgiFactory를 생성하고, 그것을 이용하여 디바이스를 생성
	void CreateCommandQueueList();		// 커맨드 큐 리스트 생성
	void CreateRtvAndDsvDescriptorHeaps();	// 서술자 힙 생성
	void CreateSwapChain();				// 스왑체인 생성
	void CreateRenderTargetViews();		// 렌더타겟뷰 생성 후 서술자 힙에 적재
	void CreateDepthStencilView();
	void CreateGraphicsRootSignature();	// 루트 시그니쳐 생성

// get set 함수
public:
	
	//const ComPtr<ID3D12Device>& GetDevice() const;
	//const ComPtr<ID3D12GraphicsCommandList>& GetCommandList() const;
	//const ComPtr<ID3D12CommandQueue>& GetCommandQueue() const;
	//const ComPtr<ID3D12RootSignature>& GetRootSignature() const;
	pair<int, int> GetClientSize();
	bool GetDrawHitBox() const;
	const shared_ptr<Scene>& GetCurrentScene() const;
	MeshManager& GetMeshManager();
	TextureManager& GetTextureManager();
	GameObjectManager& GetGameObjectManager();

	
	void FrameAdvance();
	void WaitForGpuComplete();			// GPU와 동기화하기 위한 대기
	void MoveToNextFrame();				// 다음 후면버퍼로 변경후 WaitForGpuComplete() 수행

	void ChangeSwapChainState();
	void ProcessInput();

	void PushScene(const shared_ptr<Scene>& _pScene);
	void PopScene();
	void ChangeScene(const shared_ptr<Scene>& _pScene);
	void ClearScene();

	// 서버로부터 다른 플레이어들의 정보 받아 적용하기
	void RecvWorldData();

	// 서버에서 받은 패킷으로 현재 씬에 적용
	void AddEnemy(const SC_ADD_PLAYER& _packet, bool _isNew);
	void AddMissile(const SC_ADD_MISSILE& _packet);
	void EnemyMove(const SC_MOVE_PLAYER& _packet);
	void RemoveMissile(const SC_REMOVE_MISSILE& _packet);
	void RemoveEnemy(const SC_REMOVE_PLAYER& _packet);

	void SetGameOver();
	bool IsGameOver();

	// 플레이어가 죽었을때 불리는 함수, 서버에게 removePlayer패킷을 보낸다.
	int SendPlayerRemove();

};
