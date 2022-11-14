#pragma once
#include "GameObject.h"
#include "Status.h"
#include "Camera.h"

class Player : public GameObject, public RigidBody {
private:
	// 플레이어가 죽은지를 판단
	float hp;
	bool isDead;
	weak_ptr<Camera> pCamera;
	float reloadTime;

	USHORT clientId;

public:
	Player();
	~Player();

public:
	void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	bool GetIsDead() const;

	void FireMissile(UINT& _mid, list<shared_ptr<GameObject>>& _pMissiles, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	shared_ptr<Camera> GetCamera() const;

	void SetClientID(USHORT _cid) { clientId = _cid; };
	USHORT GetClientID() const { return clientId; };
	void Animate(double _timeElapsed);
	float Hit(float _damage) { hp -= _damage; return hp; };
};
