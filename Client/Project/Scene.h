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
};


///////////////////////////////////////////////////////////////////////////////
/// PlayScene
class PlayScene : public Scene {
private:
	
	int nStage;
	float spawnTime;
	float playerHP;
	
	float keybufferTime;
	bool drawHitBoxToggle;
	
	// 현재 아군 플레이어.
	shared_ptr<Player> pPlayer;

	// 적 플레이어가 들어갈 부분. 최대 2명
	list<shared_ptr<Player>> pEnemys;

	shared_ptr<GameObject> pWater;
	
	list<shared_ptr<GameObject>> pMissiles;
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
	
	shared_ptr<TerrainMap> GetTerrain() const;
	virtual void LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};