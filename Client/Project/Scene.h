#pragma once
#include "Player.h"
#include "Light.h"
#include "Image2D.h"


class Scene {
protected:

public:
	Scene();
	virtual ~Scene();

public:
	virtual void LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ReleaseUploadBuffers() = 0;
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void CheckCollision();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	
	virtual void AddEnemy(const SC_ADD_PLAYER& _packet, bool _isNew, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void AddMissile(const SC_ADD_MISSILE& _packet, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void EnemyMove(const SC_MOVE_PLAYER& _packet) = 0;
	virtual void RemoveMissile(const SC_REMOVE_MISSILE& _packet) = 0;
	virtual void RemoveEnemy(const SC_REMOVE_PLAYER& _packet) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// PlayScene
class PlayScene : public Scene {
private:

	float playerHP;
	
	// ������ ���������� ���� ��ġ���� ����
	XMFLOAT3 prevPosition;
	XMFLOAT4 prevRotation;

	UINT missileCount;
	float keybufferTime;
	
	// ���� �Ʊ� �÷��̾�.
	shared_ptr<Player> pPlayer;

	// �� �÷��̾ �� �κ�.
	list<shared_ptr<Player>> pEnemys;

	// ���� �� ���� �̻��ϱ��� ����
	list<shared_ptr<Missile>> pMissiles;

	shared_ptr<GameObject> pWater;
	
	list<shared_ptr<Effect>> pEffects;
	
	shared_ptr<SkyBox> pSkyBox;
	
	
	shared_ptr<TerrainMap> pTerrain;
	vector<shared_ptr<BillBoard>> pBillBoards;
	unordered_map<string, shared_ptr<Image2D>> pUIs;

	ComPtr<ID3D12Resource> pLightsBuffer;
	vector<shared_ptr<Light>> pLights;
	shared_ptr<LightsMappedFormat> pMappedLights;
	
	XMFLOAT4 globalAmbient;
	shared_ptr<Camera> camera;

	XMFLOAT3 prePlayerPosition;
	XMFLOAT4 prePlayerRotation;

public:
	PlayScene(int _stageNum);
	~PlayScene() final;

public:
	void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void ReleaseUploadBuffers();
	void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	
	void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void CheckCollision() final;
	void UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;

	void AddLight(const shared_ptr<Light>& _pLight);
	void AddEnemy(const SC_ADD_PLAYER& _packet, bool _isNew, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void AddMissile(const SC_ADD_MISSILE& _packet, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void EnemyMove(const SC_MOVE_PLAYER& _packet) final;
	void RemoveMissile(const SC_REMOVE_MISSILE& _packet) final;
	void RemoveEnemy(const SC_REMOVE_PLAYER& _packet) final;

	int SendNewMissile();
	shared_ptr<TerrainMap> GetTerrain() const;
	virtual void LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	void SetPlayerClientID(USHORT _cid);
	int SendMissileRemove(UINT _missileId);

};