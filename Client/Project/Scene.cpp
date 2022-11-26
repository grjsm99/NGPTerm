#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"

#include "../../protocol.h"


Scene::Scene() {
	
}

Scene::~Scene() {
	
}

void Scene::CheckCollision() {

}

///////////////////////////////////////////////////////////////////////////////
/// PlayScene
PlayScene::PlayScene(int _stageNum) {
	globalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	keybufferTime = 0;
	playerHP = 100;
	missileCount = 0;
	// 서버에서의 초기값으로 바꾸어주자
	prevPosition = XMFLOAT3(0, 0, 0);
	prevRotation = XMFLOAT4(0, 0, 0, 1);
}

void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	// 스테이지 생성
	LoadStage(_pDevice, _pCommandList);
	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	camera->SetLocalPosition(XMFLOAT3(0.0, 10.0, -20)); // pPlayer 고정시

	camera->SetLocalRotation(Vector4::QuaternionRotation(XMFLOAT3(0,1,0), 0.0f));
	camera->UpdateLocalTransform();
	camera->UpdateWorldTransform();
	pPlayer->SetChild(camera);

	pPlayer->UpdateObject();

	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256의 배수
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	

	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

	GameObject::Init(_pDevice, _pCommandList);
}

void PlayScene::ReleaseUploadBuffers() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetMeshManager().ReleaseUploadBuffers();
	gameFramework.GetTextureManager().ReleaseUploadBuffers();
	for (auto& pUI : pUIs) {
		pUI.second->ReleaseUploadBuffers();
	}
}

PlayScene::~PlayScene() {
	pLightsBuffer->Unmap(0, NULL);
}

void PlayScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	

	GameFramework& gameFramework = GameFramework::Instance();
	
	if (_keysBuffers['E'] & 0xF0) {
		pPlayer->RotateRigid(XMFLOAT3(0, 1, 0), 90.0f, _timeElapsed);
	}
	if (_keysBuffers['Q'] & 0xF0) {
		pPlayer->RotateRigid(XMFLOAT3(0, 1, 0), -90.0f, _timeElapsed);
	}
	if (_keysBuffers['3'] & 0xF0) {
		pPlayer->RotateRigid(pPlayer->GetLocalRightVector(), 90.0f, _timeElapsed);
	}
	if (_keysBuffers['4'] & 0xF0) {
		pPlayer->RotateRigid(pPlayer->GetLocalRightVector(), -90.0f, _timeElapsed);
	}
	// 
	if (_keysBuffers['W'] & 0xF0) {
		pPlayer->MoveFrontRigid(true, _timeElapsed);
	}
	if (_keysBuffers['S'] & 0xF0) {
		pPlayer->MoveFrontRigid(false, _timeElapsed);
	}
	if (_keysBuffers['D'] & 0xF0) {
		pPlayer->MoveRightRigid(true, _timeElapsed);
	}
	if (_keysBuffers['A'] & 0xF0) {
		pPlayer->MoveRightRigid(false, _timeElapsed);
	}
	if (_keysBuffers['1'] & 0xF0) {
		pPlayer->MoveUpRigid(true, _timeElapsed);
	}
	if (_keysBuffers['2'] & 0xF0) {
		pPlayer->MoveUpRigid(false, _timeElapsed);
	}
	if (_keysBuffers['P'] & 0xF0) {
		//pPlayer->FireMissile(missileCount, pMissiles, _pDevice, _pCommandList);
		if (pPlayer->GetReloadTime() < 0) {
			SendNewMissile();
			pPlayer->SetReloadTime(0.1f);
		}
	}
	//pPlayers[0]->ApplyTransform(transform, false);
}

void PlayScene::AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	if (pPlayer->GetIsDead()) return;
	keybufferTime -= _timeElapsed;

	// 플레이어가 살아있는 경우 애니메이션을 수행
	if (!pPlayer->GetIsDead()) {
		pPlayer->Animate(_timeElapsed);
		// 패킷 send
	}		
	if(prevPosition.x != pPlayer->GetLocalPosition().x) 

	for (auto& pLight : pLights) {
		if (pLight) {
			pLight->UpdateLight();
		}
	}
	for (auto& pEffect : pEffects) {
		if (pEffect) {
			pEffect->Animate(_timeElapsed);
		}
	}
	EnterCriticalSection(&missileCS);
	for (auto& pMissile : pMissiles) {
		pMissile->Animate(_timeElapsed);
	}
	LeaveCriticalSection(&missileCS);

	// 적 플레이어의 이동은 들어온 패킷으로만 처리하기 때문에 Animate를 하지 않는다.

	for (auto& pEnemy : pEnemys) {
		pEnemy->SubInvisible(_timeElapsed);
	}
	pWater->Animate(_timeElapsed);
}

void PlayScene::CheckCollision() {

	EnterCriticalSection(&missileCS);
	
	
	// 미사일과 플레이어의 충돌체크 (CheckCollideWithMissile)
	if (pPlayer->GetIsInvisible()) {
		for (auto& pMissile : pMissiles) {
			if (pMissile->GetClientID() != pPlayer->GetClientID() && pMissile->GetObj()->GetBoundingBox().Contains(pPlayer->GetObj()->GetBoundingBox())) {
				pMissile->Remove();
				playerHP = pPlayer->Hit(5.0f);

				// 제거할 미사일의 id를 서버로 보내준다.
				if (!SendMissileRemove(pMissile->GetMissileID()))
					cout << "SendMissileRemove_Error" << endl;

				pUIs["2DUI_hp"]->SetSizeUV(XMFLOAT2(playerHP / 100.0f, 1.0f));

				shared_ptr<Effect> pEffect = make_shared<Effect>();
				pEffect->Create(0.03, 8, 8, 50, 50, pMissile->GetLocalPosition(), "Explode_8x8");

				pEffects.push_back(pEffect);
				break;
			}
		}
	}
	pMissiles.remove_if([](const shared_ptr<GameObject>& _pMissile) {
		return _pMissile->CheckRemove();
		});

	LeaveCriticalSection(&missileCS);


	EnterCriticalSection(&playerCS);
	pEnemys.remove_if([](const shared_ptr<GameObject>& pEnemy) {
		return pEnemy->CheckRemove();
		});
	LeaveCriticalSection(&playerCS);


	//pEffects.remove_if([](const shared_ptr<Effect>& pEffect) {
	//	return pEffect->CheckRemove();
	//	});
}

void PlayScene::UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 적의 월드 행렬을 리소스 내에 직접 쓴다.
	int i = 0;
	for (auto& pEnemy : pEnemys) {
		pEnemy->UpdateShaderVariableInstance(_pCommandList, i++);
	}
}

void PlayScene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	int nLight = (UINT)pLights.size();
	for (int i = 0; i < nLight; ++i) {
		
		memcpy(&pMappedLights->lights[i], pLights[i].get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));

	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

}

void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {	

	GameFramework& gameFramework = GameFramework::Instance();
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);
	if (pPlayer->GetIsDead()) {
		Image2D::GetShader()->PrepareRender(_pCommandList);
		pUIs["2DUI_gameover"]->Render(_pCommandList);
		return;
	}
	//UpdateShaderVariables(_pCommandList);

	Image2D::GetShader()->PrepareRender(_pCommandList);
	pUIs["2DUI_hpbar"]->Render(_pCommandList);
	pUIs["2DUI_hp"]->Render(_pCommandList);
	
	TerrainMesh::GetShader()->PrepareRender(_pCommandList);
	pTerrain->Render(_pCommandList);
	
	BillBoardMesh::GetShader()->PrepareRender(_pCommandList);
	for (auto& billboard : pBillBoards) {
		billboard->Render(_pCommandList);
	}
	for (auto& pEffect : pEffects) {
		pEffect->Render(_pCommandList);
	}
	
	Mesh::GetShader()->PrepareRender(_pCommandList);
	pPlayer->Render(_pCommandList);	
	

	EnterCriticalSection(&missileCS);
	for (auto& pMissile : pMissiles) {
		if (pMissile) pMissile->Render(_pCommandList);
	}
	LeaveCriticalSection(&missileCS);

	
	EnterCriticalSection(&playerCS);
	for (auto& pEnemy : pEnemys) {
		if (pEnemy) pEnemy->Render(_pCommandList);
	}
	LeaveCriticalSection(&playerCS);

	WaterMesh::GetShader()->PrepareRender(_pCommandList);
	pWater->Render(_pCommandList);
	

	SkyBoxMesh::GetShader()->PrepareRender(_pCommandList);
	pSkyBox->Render(_pCommandList, camera);
}

void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

void PlayScene::AddEnemy(const SC_ADD_PLAYER& _packet, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	shared_ptr<Player> pEnemy = make_shared<Player>();
	pEnemy->Create("Gunship", _pDevice, _pCommandList);
	//pEnemy->SetLocalScale(XMFLOAT3(22,22,22));

	pEnemy->SetLocalPosition(_packet.localPosition);
	pEnemy->SetLocalRotation(_packet.localRotation);
	pEnemy->UpdateObject();
	pEnemy->SetClientID(_packet.client_id);

	EnterCriticalSection(&playerCS);
	pEnemys.push_back(pEnemy);
	LeaveCriticalSection(&playerCS);
}

void PlayScene::AddMissile(const SC_ADD_MISSILE& _packet, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	shared_ptr<Missile> pMissile = make_shared<Missile>();
	pMissile->Create(_packet.client_id, _packet.missile_id, _packet.position, _packet.rotation, _pDevice, _pCommandList);
	
	EnterCriticalSection(&missileCS);
	pMissiles.push_back(pMissile);
	LeaveCriticalSection(&missileCS);
}

void PlayScene::EnemyMove(const SC_MOVE_PLAYER& _packet)
{
	EnterCriticalSection(&playerCS);
	auto target = find_if(pEnemys.begin(), pEnemys.end(), [_packet](const shared_ptr<Player>& _p) { return _p->GetClientID() == _packet.client_id; });
	// 추후 각 플레이어마다 임계영역을 따로 만들어 설정해주도록 바꿀 예정
	
	// 받은 플레이어가 이미 죽은 상태면 패킷을 처리하지 않는다.
	if (target != pEnemys.end()) {
		(*target)->SetLocalPosition(_packet.localPosition);
		(*target)->SetLocalRotation(_packet.localRotation);
		(*target)->UpdateObject();
	}	
	LeaveCriticalSection(&playerCS);
}

void PlayScene::RemoveMissile(const SC_REMOVE_MISSILE& _packet)
{
	EnterCriticalSection(&missileCS);
	auto target = lower_bound(pMissiles.begin(), pMissiles.end(), _packet.missile_id, [](const shared_ptr<Missile>& _p, UINT _mid) { return _p->GetMissileID() < _mid; });
	// 그 미사일이 아직 남아있을 때 ( 미사일이 이미 시간이 지나 사라졌을 수 있다 )
	if (target != pMissiles.end()) {
		(*target)->CheckRemove();
	}
	LeaveCriticalSection(&missileCS);
}

void PlayScene::RemoveEnemy(const SC_REMOVE_PLAYER& _packet)
{
	EnterCriticalSection(&playerCS);
	auto target = find_if(pEnemys.begin(), pEnemys.end(), [_packet](const shared_ptr<Player>& _p) { return _p->GetClientID() == _packet.client_id; });
	(*target)->CheckRemove();
	LeaveCriticalSection(&playerCS);
}

int PlayScene::SendNewMissile() {
	//그 쓰레드 함수마다 cid가 있으므로  타입만 보내주면 된다.
	int result;
	CS_ADD_MISSILE packet;
	packet.type = 1;
	result = send(serverSock, (char*)&packet, sizeof(packet), 0);
	if (result == SOCKET_ERROR) {
		err_display("SendNewMissile()");
		return -1;
	}
	return result;
}

shared_ptr<TerrainMap> PlayScene::GetTerrain() const {
	return pTerrain;
}

void PlayScene::LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 씬에 그려질 오브젝트들을 전부 빌드.
	
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetGameObjectManager().GetGameObject("Missile", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().GetGameObject("Apache", _pDevice, _pCommandList);
	Effect::CreateBaseMesh(_pDevice, _pCommandList);
	Effect::LoadEffect("Explode_8x8", _pDevice, _pCommandList);
	
	pPlayer = make_shared<Player>();
	pPlayer->Create("Gunship", _pDevice, _pCommandList);
	pPlayer->UpdateObject();
	shared_ptr<Light> baseLight = make_shared<Light>();
	baseLight->lightType = 3;
	baseLight->position = XMFLOAT3(0, 500, 0);
	baseLight->direction = XMFLOAT3(0, -1, 0);
	baseLight->diffuse = XMFLOAT4(0.1, 0.1, 0.1, 1);
	AddLight(baseLight);
	
	
	// size, startpos, uvsize
	// pos = x,y -> 0~2
	
	string name = "2DUI_hp";
	shared_ptr<Image2D> phpUI = make_shared<Image2D>(name, XMFLOAT2(0.6, 0.2), XMFLOAT2(1.385,1.75), XMFLOAT2(1,1), _pDevice, _pCommandList);
	pUIs[name] = phpUI;
	
	name = "2DUI_hpbar";
	phpUI = make_shared<Image2D>(name, XMFLOAT2(0.7, 0.3), XMFLOAT2(1.3, 1.7), XMFLOAT2(1, 1), _pDevice, _pCommandList);
	pUIs[name] = phpUI;
	
	name = "2DUI_gameover";
	phpUI = make_shared<Image2D>(name, XMFLOAT2(2, 2), XMFLOAT2(0,0), XMFLOAT2(1, 1), _pDevice, _pCommandList);
	pUIs[name] = phpUI;

	XMFLOAT3 size = XMFLOAT3(2056, 2056, 2056);
	pWater = make_shared<Water>(size, WATER_HEIGHT, _pDevice, _pCommandList);
	


	XMFLOAT3 tScale = XMFLOAT3(8.0f, 2.0f, 8.0f);
	XMFLOAT4 tColor = XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f);
	
	pTerrain = make_shared<TerrainMap>(_pDevice, _pCommandList, _T("Texture/HeightMap.raw"), 257, 257, 257, 257, tScale, tColor);
	RigidBody::pTerrain = pTerrain;

	shared_ptr<BillBoard> billboard; 
	
	billboard = make_shared<BillBoard>();
	billboard->Create("Flower01", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);

	billboard = make_shared<BillBoard>();
	billboard->Create("Flower02", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);
	
	billboard = make_shared<BillBoard>();
	billboard->Create("Grass01", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);
	
	billboard = make_shared<BillBoard>();
	billboard->Create("Tree02", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);

	billboard = make_shared<BillBoard>();
	billboard->Create("Tree03", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);

	pSkyBox = make_shared<SkyBox>();
	pSkyBox->Create(_pDevice, _pCommandList);
}

void PlayScene::SetPlayerClientID(USHORT _cid) {
	pPlayer->SetClientID(_cid);
}

// 플레이어가 죽었을때 불리는 함수, 서버에게 removePlayer패킷을 보낸다.
bool PlayScene::SendPlayerRemove() {
	CS_REMOVE_PLAYER removePlayerPacket;
	int result = send(serverSock, (char*) &removePlayerPacket, sizeof(removePlayerPacket), 0);

	if (result == SOCKET_ERROR) {
		err_display("send()");
		return result;
	}
}


// 특정 클라이언트가 어떤 미사일과 충돌 시 충돌된 missile id를 모두에게 Send 후 모두 성공 시 true를 반환한다
bool PlayScene::SendMissileRemove(UINT _mid)
{
	CS_REMOVE_MISSILE packet;
	int retval = send(serverSock, (char*)&packet, sizeof(packet), 0);

	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return false;
	}

	return true;
}