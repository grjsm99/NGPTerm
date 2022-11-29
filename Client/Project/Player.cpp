#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"

Player::Player() {
	isPlayer = false;
	hp = 100.0f;
	isDead = false;
	moveSpeed = 150.0f;
	reloadTime = 0.0f;
	clientId = 0;
	isInvincible = true;
	invincibleTime = 2.0f;
}

Player::~Player() {

}

void Player::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);
	invincibleTime = 2.0f;
	GameFramework& gameFramework = GameFramework::Instance();
	
	self = shared_from_this();
	SetLocalPosition(XMFLOAT3(500, 200, 500));
	name = "플레이어";

	//pLight = make_shared<Light>(shared_from_this());
	// 클래스 상속 관계에서 포인터 형 변환시 dynamic_cast(런타임 이후 동작)을 사용하자
	//auto playScene = dynamic_pointer_cast<PlayScene>(gameFramework.GetCurrentScene());
	//playScene->AddLight(pLight);

	UpdateObject();
	
}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::FireMissile(UINT& _mid, list<shared_ptr<Missile>>& _pMissiles, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (reloadTime > 0) return;
	reloadTime = 0.1f;
	shared_ptr<Missile> pMissile = make_shared<Missile>();
	
	pMissile->Create(clientId, ++_mid, GetWorldPosition(), GetLocalRotate(), _pDevice, _pCommandList);
	
	// (수정) 동기화 및 서버로 전송 추가 필요
	cout << _mid << "\n";
	
	_pMissiles.push_back(pMissile);
}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	float isInv = 0;
	if (isInvincible) {
		isInv = 255;
	}
	_pCommandList->SetGraphicsRoot32BitConstants(6, 1, &isInv, 0);

	GameObject::Render(_pCommandList);
}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}

void Player::Animate(double _timeElapsed) {

	if (isInvincible) {
		invincibleTime -= _timeElapsed;
		if (invincibleTime < FLT_EPSILON) {
			isInvincible = false;
		}
	}
	//if (!isPlayer) return;
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
	
	// 비행기가 기울어져 있으면 수평으로 날수있게 회전한다.
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
	// maxSpeed
	if (!CheckCollisionWithTerrain(shared_from_this()))
	{
		moveVector = XMFLOAT3(0, 0, 0);
		// 등속운동으로 변경
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

bool Player::SendPlayerMove() {
	CS_MOVE_PLAYER packet;
	packet.localPosition = localPosition;
	packet.localRotation = localRotation;
	cout << localPosition << " , " << localRotation << "\n";
	int retval = send(serverSock, (char*)&packet, sizeof(packet), 0);

	if (retval == SOCKET_ERROR) {
		err_display("Error SendPlayerMove()");
		return false;
	}

	return true;
}
