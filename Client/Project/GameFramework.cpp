	#include "stdafx.h"
#include "GameFramework.h"


unique_ptr<GameFramework> GameFramework::spInstance;


void GameFramework::Create(HINSTANCE _hInstance, HWND _hMainWnd) {
	
	if (!spInstance) {		// ������ ��ũ �ν��Ͻ��� ���������� ���� ��� 
		spInstance.reset(new GameFramework());
		GameFramework& gameFramework = *spInstance;

		gameFramework.instanceHandle = _hInstance;
		gameFramework.windowHandle = _hMainWnd;

		gameFramework.CreateDirect3DDevice();    // ������� ����̽��� �����ؾ� ��� ��⿭�̳� ������ �� ���� ������ �� �ִ�.
		gameFramework.CreateCommandQueueList();
		gameFramework.CreateRtvAndDsvDescriptorHeaps();
		gameFramework.CreateSwapChain();    // DxgiFactory, CommandQueue, RtvDescriptorHeap �� �̸� ������� �־�� �Ѵ�.
		gameFramework.CreateRenderTargetViews();
		gameFramework.CreateDepthStencilView();
		gameFramework.CreateGraphicsRootSignature();

		CoInitialize(NULL);
		gameFramework.pCommandList->Reset(gameFramework.pCommandAllocator.Get(), NULL);

		// ���̴� ����
		Mesh::MakeShader(gameFramework.pDevice, gameFramework.pRootSignature);
		HitBoxMesh::MakeShader(gameFramework.pDevice, gameFramework.pRootSignature);
		TerrainMesh::MakeShader(gameFramework.pDevice, gameFramework.pRootSignature);
		BillBoardMesh::MakeShader(gameFramework.pDevice, gameFramework.pRootSignature);
		SkyBoxMesh::MakeShader(gameFramework.pDevice, gameFramework.pRootSignature);
		WaterMesh::MakeShader(gameFramework.pDevice, gameFramework.pRootSignature);
		Image2D::MakeShader(gameFramework.pDevice, gameFramework.pRootSignature);
		
		// ���ʾ� ����
		shared_ptr<Scene> startScene = make_shared<PlayScene>(1);
		gameFramework.PushScene(startScene);

		// ��Ʈ�ڽ��� �޽� ����
		gameFramework.meshManager.GetHitBoxMesh().Create(gameFramework.pDevice, gameFramework.pCommandList);
		
		gameFramework.pCommandList->Close();
		
		vector<ComPtr<ID3D12CommandList>> pCommandLists = { gameFramework.pCommandList.Get() };
		gameFramework.pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());

		gameFramework.WaitForGpuComplete();
		startScene->ReleaseUploadBuffers();

		// RecvWorldData();

		gameFramework.gameTimer.Reset();    // Ÿ�̸� ����
	}

}

void GameFramework::Destroy() {

	if (spInstance) {
		GameFramework& gameFramework = *spInstance;

		CloseHandle(gameFramework.fenceEvent);
	}
}

GameFramework& GameFramework::Instance() {
	return *spInstance;
}

pair<int, int> GameFramework::GetClientSize() {
	return { clientWidth , clientHeight };
}

bool GameFramework::GetDrawHitBox() const {
	return drawHitBox;
}

GameFramework::GameFramework() {
	instanceHandle = NULL;
	windowHandle = NULL;

	// MSAA ���� ���ø�
	msaa4xEnable = false;
	msaa4xLevel = 0;

	drawHitBox = true;
	clientHeight = 1920;
	clientWidth = 1080;
	dsvDescriptorIncrementSize = 0;
	fenceValues.fill(0);
	rtvDescriptorIncrementSize = 0;

	swapChainBufferCurrentIndex = 0;
	fenceEvent = NULL;
}

GameFramework::~GameFramework() {

}



void GameFramework::CreateDirect3DDevice() {
	HRESULT hResult;

	UINT nDxgiFactoryFlag = 0;

	// pDxgiFactory�� ���丮 ����
	hResult = CreateDXGIFactory2(nDxgiFactoryFlag, __uuidof(IDXGIFactory4), (void**) &pDxgiFactory);

	ComPtr<IDXGIAdapter1> pAdapter = NULL;

	for (int i = 0; DXGI_ERROR_NOT_FOUND != pDxgiFactory->EnumAdapters1(i, &pAdapter); ++i) {
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pAdapter->GetDesc1(&dxgiAdapterDesc);		// ������� ������ �����´�.

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { // ����Ͱ� ����Ʈ�����̸� �ǳʶ�.
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&pDevice))) { // ������ ����� ã�� ���
			break;
		}
	}

	if (!pAdapter) {// ������ ����Ͱ� ���� ��� 
		pDxgiFactory->EnumWarpAdapter(__uuidof(IDXGIFactory4), (void**)&pAdapter);
		hResult = D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&pDevice);
	}


	::gnCbvSrvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	// ���� ���ø� ����
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevel;
	msaaQualityLevel.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaQualityLevel.SampleCount = 4;
	msaaQualityLevel.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevel.NumQualityLevels = 0;
	hResult = pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevel, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	msaa4xLevel = msaaQualityLevel.NumQualityLevels;

	// 1 �ʰ��Ͻ� msaa ����
	msaa4xEnable = (msaa4xLevel > 1) ? true : false;

	// �潺 
	hResult = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&pFence);

	for (UINT64 fenceValue : fenceValues) {
		fenceValue = 0;
	}

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void GameFramework::CreateCommandQueueList() {
	HRESULT hResult;

	// ��� ��⿭ ����
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = pDevice->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&pCommandQueue);

	// ��� �Ҵ��� ����
	hResult = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&pCommandAllocator);

	// ��� ����Ʈ ����
	hResult = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), NULL, __uuidof(ID3D12CommandList), (void**)&pCommandList);
	hResult = pCommandList->Close();

}

void GameFramework::CreateRtvAndDsvDescriptorHeaps() {
	// ����Ÿ�� ������ �� ����
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	ZeroMemory(&descriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	descriptorHeapDesc.NumDescriptors = nSwapChainBuffer;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pRtvDescriptorHeap);
	rtvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ���� ���ٽ� ������ �� ����
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&pDsvDescriptorHeap);
	dsvDescriptorIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}

void GameFramework::CreateSwapChain()
{
	RECT rcClient;
	GetClientRect(windowHandle, &rcClient);    // Ŭ���̾�Ʈ ũ�⸦ rcClient�� ����
	clientWidth = rcClient.right - rcClient.left;
	clientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = nSwapChainBuffer;    // ����ü��(���� Ÿ��) ������ ����
	dxgiSwapChainDesc.BufferDesc.Width = clientWidth;    // ������ Ŭ���̾�Ʈ ������ ���� ũ�� 
	dxgiSwapChainDesc.BufferDesc.Height = clientHeight;    // ������ Ŭ���̾�Ʈ ������ ���� ũ�� 
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // �ȼ��� RGBA�� 8bit ���·� �׸���
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;    // RefreshRate : ȭ�� ���ŷ�(������) //����
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;    // �и�
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // ��¿� ���۷� ���ڴٰ� �����ϴ°�.        �б�����, ���̴��� �Է����� ���, ����, ������ȭ ���� ���� ���°� �ִ�.
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;    // ���� ���ۿ� �ĸ���۸� ��ü�ϰ� ���� ������ ���(���� ���� ������ �ӵ��� ������)�Ѵ�.
	dxgiSwapChainDesc.OutputWindow = windowHandle;    // ����� ������
	dxgiSwapChainDesc.SampleDesc.Count = (msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (msaa4xEnable) ? (msaa4xLevel - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;    // â��带 ������?
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = pDxgiFactory->CreateSwapChain(pCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)pDxgiSwapChain.GetAddressOf());

	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = pDxgiFactory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

}

void GameFramework::CreateRenderTargetViews() {
	HRESULT hResult;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();    // ������ ���� ���� �����ּҸ� �����´�.
	for (int i = 0; i < nSwapChainBuffer; ++i) {
		hResult = pDxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pRenderTargetBuffers[i]);    // ����Ÿ�� ���۸� �����Ѵ�.
		pDevice->CreateRenderTargetView(pRenderTargetBuffers[i].Get(), NULL, rtvCPUDescriptorHandle);    // ����Ÿ�� �並 ������ ���� ����(����)
		rtvCPUDescriptorHandle.ptr += rtvDescriptorIncrementSize;    // ������ �ּҷ� �̵�
	}
}

void GameFramework::CreateDepthStencilView() {
	HRESULT hResult;
	// ����-���ٽ� ����(���ҽ�)�� ����� ���� ������ �Է�
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = clientWidth;
	resourceDesc.Height = clientHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = (msaa4xEnable) ? 4 : 1;
	resourceDesc.SampleDesc.Quality = (msaa4xEnable) ? (msaa4xLevel - 1) : 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties;
	::ZeroMemory(&heapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// ����-���ٽ� ����(���ҽ�) ����
	hResult = pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, __uuidof(ID3D12Resource), (void**)&pDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	::ZeroMemory(&depthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pDevice->CreateDepthStencilView(pDepthStencilBuffer.Get(), &depthStencilViewDesc, dsvCPUDescriptorHandle);    // ����-���ٽ� �並 ������ ���� ����(����) (��==������?)
}

void GameFramework::CreateGraphicsRootSignature() {
	HRESULT hResult;

	D3D12_DESCRIPTOR_RANGE pDescriptorRanges[2];

	pDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[0].NumDescriptors = 1;
	pDescriptorRanges[0].BaseShaderRegister = 5;	// t5 = albedoMap
	pDescriptorRanges[0].RegisterSpace = 0;
	pDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pDescriptorRanges[1].NumDescriptors = 2;
	pDescriptorRanges[1].BaseShaderRegister = 6;	// t6 = TerrainBaseMap, t7 = TerrainDetailMap
	pDescriptorRanges[1].RegisterSpace = 0;
	pDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	D3D12_ROOT_PARAMETER pRootParameters[6];

	pRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[0].Descriptor.ShaderRegister = 1; //Camera //shader.hlsl�� �������� ��ȣ (���� register(b1) )
	pRootParameters[0].Descriptor.RegisterSpace = 0;
	pRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pRootParameters[1].Constants.Num32BitValues = 16;
	pRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pRootParameters[1].Constants.RegisterSpace = 0;
	pRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[2].Descriptor.ShaderRegister = 3; // Lights
	pRootParameters[2].Descriptor.RegisterSpace = 0;
	pRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pRootParameters[3].Descriptor.ShaderRegister = 4; // Material
	pRootParameters[3].Descriptor.RegisterSpace = 0;
	pRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[4].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[0];
	pRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// albedoMap
	
	pRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pRootParameters[5].DescriptorTable.pDescriptorRanges = &pDescriptorRanges[1];
	pRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// texture Map
	
	
	// ���� ���÷�
	D3D12_STATIC_SAMPLER_DESC samplerDesc[2];
	::ZeroMemory(samplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC) * 2);
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	// ���� ���͸�
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	// �ؽ�ó Ÿ�ϸ��� �ݺ�
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].MipLODBias = 0;
	samplerDesc[0].MaxAnisotropy = 1;
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc[0].MinLOD = 0.0f;
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[0].ShaderRegister = 0;	// s0
	samplerDesc[0].RegisterSpace = 0;
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	
	samplerDesc[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].MipLODBias = 0;
	samplerDesc[1].MaxAnisotropy = 1;
	samplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc[1].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc[1].MinLOD = 0.0f;
	samplerDesc[1].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[1].ShaderRegister = 1;	// s1
	samplerDesc[1].RegisterSpace = 0;
	samplerDesc[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	::ZeroMemory(&rootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	rootSignatureDesc.NumParameters = _countof(pRootParameters);
	rootSignatureDesc.pParameters = pRootParameters;
	rootSignatureDesc.NumStaticSamplers = _countof(samplerDesc);
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.Flags = rootSignatureFlags;

	ComPtr<ID3DBlob> pSignatureBlob = NULL;
	ComPtr<ID3DBlob> pErrorBlob = NULL;
	hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignatureBlob, &pErrorBlob);
	
	hResult = pDevice->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(), pSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pRootSignature);
}

// get, set �Լ�

//const ComPtr<ID3D12Device>& GameFramework::GetDevice() const {
//	return pDevice;
//}
//const ComPtr<ID3D12GraphicsCommandList>& GameFramework::GetCommandList() const {
//	return pCommandList;
//}
//const ComPtr<ID3D12CommandQueue>& GameFramework::GetCommandQueue() const {
//	return pCommandQueue;
//}
//const ComPtr<ID3D12RootSignature>& GameFramework::GetRootSignature() const {
//	return pRootSignature;
//}
MeshManager& GameFramework::GetMeshManager() {
	return meshManager;
}

TextureManager& GameFramework::GetTextureManager() {
	return textureManager;
}

GameObjectManager& GameFramework::GetGameObjectManager() {
	return gameObjectManager;
}
const shared_ptr<Scene>& GameFramework::GetCurrentScene() const {
	//if (!pScenes.empty()) {
		return pScenes.top();
	//}
}


void GameFramework::FrameAdvance() {

	gameTimer.Tick(.0f);
	
	if (pScenes.empty()) {
		// ���ʾ� ����
		shared_ptr<Scene> startScene = make_shared<PlayScene>(1);
		PushScene(startScene);
	}
	
	ProcessInput();
	

	if (!pScenes.empty()) {	// �� ����(�ִϸ���Ʈ). ������ �� �� ���ҿ� ���� ����
		pScenes.top()->AnimateObjects(gameTimer.GetTimeElapsed(), pDevice, pCommandList);
		pScenes.top()->CheckCollision();
		// ���� ������Ʈ �浹ó�� [����]
	}


	
	// ��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�.
	HRESULT hResult = pCommandAllocator->Reset();
	
	
	hResult = pCommandList->Reset(pCommandAllocator.Get(), NULL);


	// ���� ���� Ÿ�ٿ� ���� Present�� �����⸦ ��ٸ�.  (PRESENT = ������Ʈ ����, RENDER_TARGET = ���� Ÿ�� ����
	D3D12_RESOURCE_BARRIER resourceBarrier;
	ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = pRenderTargetBuffers[swapChainBufferCurrentIndex].Get();
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &resourceBarrier);
	
	// ���� ���� Ÿ�� ���� CPU �ּ� ���
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCPUDescriptorHandle.ptr += (rtvDescriptorIncrementSize * swapChainBufferCurrentIndex);

	//����-���ٽ� �������� CPU �ּҸ� ����Ѵ�. 
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = pDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//���� Ÿ�� ��(������)�� ����-���ٽ� ��(������)�� ���-���� �ܰ�(OM)�� �����Ѵ�. 
	pCommandList->OMSetRenderTargets(1, &rtvCPUDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);

	float pClearColor[4] = {0.0f, 0.1f, 0.3f, 1.0f};
	pCommandList->ClearRenderTargetView(rtvCPUDescriptorHandle, pClearColor, 0, NULL);

	//���ϴ� ������ ����-���ٽ�(��)�� �����. 
	pCommandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//�׷��� ��Ʈ �ñ׳��ĸ� ���������ο� ����(����)�Ѵ�.
	pCommandList->SetGraphicsRootSignature(pRootSignature.Get());

	//�� ������  (Scene���� ī�޶�, �÷��̾ �����Ѵ�.)
	if (!pScenes.empty()) {
		pScenes.top()->Render(pCommandList);
	}

	// ���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���.
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pCommandList->ResourceBarrier(1, &resourceBarrier);
	//��� ����Ʈ�� ���� ���·� �����. 
	hResult = pCommandList->Close();
	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�. 
	vector<ComPtr<ID3D12CommandList>> pCommandLists = { pCommandList.Get() };
	pCommandQueue->ExecuteCommandLists(1, pCommandLists.data()->GetAddressOf());


	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���. 
	WaitForGpuComplete();

	//	����ü���� ������Ʈ�Ѵ�.
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	pDxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);

	// ���� ���������� �̵�, (���� ���۷� �̵�)
	MoveToNextFrame();

	// FPS ǥ��
	wstring titleString = L"FPS : " + to_wstring(gameTimer.GetFPS());
	SetWindowText(windowHandle, (LPCWSTR)titleString.c_str());
}

void GameFramework::WaitForGpuComplete() {
	UINT64 fenceValue = ++fenceValues[swapChainBufferCurrentIndex]; // ���� ������ �潺���� ����
	HRESULT hResult = pCommandQueue->Signal(pFence.Get(), fenceValue);
	if (pFence->GetCompletedValue() < fenceValue) {
		hResult = pFence->SetEventOnCompletion(fenceValue, fenceEvent);	
		WaitForSingleObject(fenceEvent, INFINITE);					// GPU�� ó���� ������ ���� ��ٸ�
	}
}

void GameFramework::MoveToNextFrame() {
	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex(); // ���� �ĸ� ���۷� ����
	WaitForGpuComplete();
}

void GameFramework::ChangeSwapChainState() {
	
	WaitForGpuComplete();
	BOOL fullScreenState;

	// ���� ��� ����
	pDxgiSwapChain->GetFullscreenState(&fullScreenState, NULL);
	pDxgiSwapChain->SetFullscreenState(~fullScreenState, NULL);


	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = clientWidth;
	dxgiTargetParameters.Height = clientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	pDxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
	for (int i = 0; i < nSwapChainBuffer; i++) {
		if (pRenderTargetBuffers[i]) {
			pRenderTargetBuffers[i].Reset();
		}
	}

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	pDxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	pDxgiSwapChain->ResizeBuffers(nSwapChainBuffer, clientWidth,
		clientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	swapChainBufferCurrentIndex = pDxgiSwapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetViews();
}


void GameFramework::ProcessInput() {

	static array<UCHAR, 256> keysBuffers;
	bool processedByScene = false;

	if (GetKeyboardState((PBYTE)keysBuffers.data())) {	// Ű����� ���� �Էµ����͸� ������
		if (keysBuffers['F'] & 0xF0) {
			//ChangeSwapChainState();
		}
		// �Ͻ�����
		if (keysBuffers['P'] & 0xF0) {
			//PushScene(make_shared<Scene>("pause"));
		}
		// �����
		if (keysBuffers['R'] & 0xF0) {
			//PopScene();
		}

		// ���� Ű�����Է� ó��
		if (!pScenes.empty()) {
			pScenes.top()->ProcessKeyboardInput(keysBuffers, gameTimer.GetTimeElapsed(), pDevice, pCommandList);
		}
	}
}

void GameFramework::PushScene(const shared_ptr<Scene>& _pScene) {
	pScenes.push(_pScene);
	pScenes.top()->Init(pDevice, pCommandList);
}
void GameFramework::PopScene() {
	if (!pScenes.empty()) {
		pScenes.pop();
	}
}
void GameFramework::ChangeScene(const shared_ptr<Scene>& _pScene) {
	PopScene();
	PushScene(_pScene);
}
void GameFramework::ClearScene() {

	while (!pScenes.empty()) {
			pScenes.pop();
	}
}

//////////////////////

void GameFramework::AddPlayer(const SC_ADD_PLAYER& _packet) {
	if (!pScenes.empty()) pScenes.top()->AddPlayer(_packet, pDevice, pCommandList);
}

void GameFramework::AddMissile(const SC_ADD_MISSILE& _packet) {
	if (!pScenes.empty()) pScenes.top()->AddMissile(_packet, pDevice, pCommandList);
}

void GameFramework::EnemyMove(const SC_MOVE_PLAYER& _packet) {
	if (!pScenes.empty()) pScenes.top()->EnemyMove(_packet);
}