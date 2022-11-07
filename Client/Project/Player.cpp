#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"



Player::Player() {
	hp = 100.0f;
	isDead = false;
	moveSpeed = 0.02f;
	reloadTime = 0.0f;
}

Player::~Player() {

}

void Player::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);
	
	GameFramework& gameFramework = GameFramework::Instance();
	
	self = shared_from_this();
	SetLocalPosition(XMFLOAT3(500, 200, 500));
	name = "�÷��̾�";

	//pLight = make_shared<Light>(shared_from_this());
	// Ŭ���� ��� ���迡�� ������ �� ��ȯ�� dynamic_cast(��Ÿ�� ���� ����)�� �������
	//auto playScene = dynamic_pointer_cast<PlayScene>(gameFramework.GetCurrentScene());
	//playScene->AddLight(pLight);

	UpdateObject();
	
}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::FireMissile(list<shared_ptr<GameObject>>& _pMissiles, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (reloadTime > 0) return;
	reloadTime = 0.1f;
	shared_ptr<Missile> pMissileLeft = make_shared<Missile>();
	shared_ptr<Missile> pMissileRight = make_shared<Missile>();
	XMFLOAT3 dist = GetLocalRightVector();
	dist = Vector3::ScalarProduct(dist, 2.0f);
	XMFLOAT3 left = Vector3::Subtract(GetWorldPosition(), dist);
	XMFLOAT3 right = Vector3::Subtract(GetWorldPosition(), Vector3::ScalarProduct(dist, -1.0f));
	pMissileLeft->Create(left, GetLocalRotate(), _pDevice, _pCommandList);
	pMissileRight->Create(right, GetLocalRotate(), _pDevice, _pCommandList);
	_pMissiles.push_back(pMissileLeft);
	_pMissiles.push_back(pMissileRight);
}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Render(_pCommandList);
}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}

void Player::Animate(double _timeElapsed) {
	if (isDead) return;
	prevWorld = worldTransform;
	
	if (hp < 0) isDead = true;
	reloadTime -= _timeElapsed;
	
	if(pChildren[0]) pChildren[0]->Animate(_timeElapsed);

	if (rotateAngle != 0.0f)
	{
		Rotate(rotateAxis, rotateAngle, _timeElapsed);
	}
	Move(moveVector, _timeElapsed);
	//UpdateObject();
	
	// ����Ⱑ ������ ������ �������� �����ְ� ȸ���Ѵ�.
	XMFLOAT3 up = GetLocalUpVector();

	XMFLOAT3 baseUp = XMFLOAT3(0, 1, 0);
	float angle2 = Vector3::Angle(up, baseUp);
	XMFLOAT3 axis2 = Vector3::CrossProduct(up, baseUp);
	//RotateRigid(axis2, angle2, _timeElapsed);
	if (angle2 != 0.0f)
	{
		Rotate(axis2, angle2, 1);
	}
	UpdateObject();

	if (!CheckCollisionWithTerrain(shared_from_this()))
	{
		moveVector = Vector3::ScalarProduct(moveVector, 0.98f);
	}
	else {
		worldTransform = prevWorld;
		XMFLOAT3 pVec = Vector3::ScalarProduct(moveVector, -1.0f);
		pVec.y += 0.1f;
		Move(pVec, _timeElapsed);
		Rotate(rotateAxis, -rotateAngle, _timeElapsed);
		moveVector = XMFLOAT3(0, 0, 0);
		UpdateObject();
	}
	
	rotateAngle = 0.0f;
	//InitVector();

}