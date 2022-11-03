#include "stdafx.h"
#include "GameObject.h"

shared_ptr<TerrainMap> RigidBody::pTerrain;

RigidBody::RigidBody() {
	acc = XMFLOAT3(0,0,0);
	moveSpeed = 0.0f;
	moveVector = XMFLOAT3(0, 0, 0);
	rotateAxis = XMFLOAT3(0, 1, 0);
	rotateAngle = 0.0f;
}


RigidBody::~RigidBody() {

}


void RigidBody::InitVector() {
	moveVector = XMFLOAT3(0, 0, 0);
	rotateAxis = XMFLOAT3(0, 1, 0);
	rotateAngle = 0.0f;
}

void RigidBody::Jump(float _power) {

}

bool RigidBody::CheckCollisionWithTerrain(const shared_ptr<GameObject>& _obj) {
	if (_obj->isOOBBCover) {
		auto boundingBox = _obj->GetBoundingBox();

		float cx = boundingBox.Center.x;
		float cz = boundingBox.Center.z;
		
		float minx = cx - boundingBox.Extents.x;
		float maxx = cx + boundingBox.Extents.x;
		float minz = cz - boundingBox.Extents.z;
		float maxz = cz + boundingBox.Extents.z;


		float height = boundingBox.Center.y - boundingBox.Extents.y;
		int checkPrecision = 3;
		for (float x = minx; x < maxx; x = min(maxx, x + checkPrecision)) {
			for (float z = minz; z < maxz; z = min(maxz, z + checkPrecision)) {

				if (pTerrain->GetHeight(x, z) > height) {
					return true;
				}
			}
		}
		return false;
	}
	for (const auto& pChild : _obj->pChildren) {
		return CheckCollisionWithTerrain(pChild);
	}
	return false;
}



void RigidBody::RotateRigid(const XMFLOAT3& _axis, float _angle, float _timeElapsed) {
	// rotate axis, angle ����
	shared_ptr<GameObject> selfLock = self.lock();
	rotateAxis = _axis;
	rotateAngle = _angle;
	
	//selfLock->SetLocalRotation(Vector4::QuaternionMultiply(selfLock->GetLocalRotate(), Vector4::QuaternionRotation(_axis, _angle)));
}

void RigidBody::MoveFrontRigid(bool _isfront, float _timeElapsed) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 frontVector = selfLock->GetLocalLookVector();
	frontVector = Vector3::Normalize(frontVector);	// �������ͷ� �ٲ���
	if(_isfront) frontVector = Vector3::ScalarProduct(frontVector, moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	else frontVector = Vector3::ScalarProduct(frontVector, -moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	
	moveVector = Vector3::Add(moveVector, frontVector);
}

void RigidBody::MoveRightRigid(bool _isright, float _timeElapsed) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 rightVector = selfLock->GetLocalRightVector();
	rightVector = Vector3::Normalize(rightVector);	// �������ͷ� �ٲ���
	if(_isright) rightVector = Vector3::ScalarProduct(rightVector, moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	else rightVector = Vector3::ScalarProduct(rightVector, -moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	
	moveVector = Vector3::Add(moveVector, rightVector);
}


void RigidBody::MoveUpRigid(bool _isup, float _timeElapsed) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 upVector = selfLock->GetLocalUpVector();
	upVector = Vector3::Normalize(upVector);	// �������ͷ� �ٲ���
	if (_isup) upVector = Vector3::ScalarProduct(upVector, moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	else upVector = Vector3::ScalarProduct(upVector, -moveSpeed);	// �̵��Ÿ���ŭ �����ش�.
	
	moveVector = Vector3::Add(moveVector, upVector);
}

void RigidBody::KnockBack(const XMFLOAT3& _vector) {
	shared_ptr<GameObject> selfLock = self.lock();
	XMFLOAT3 knockVector = Vector3::ScalarProduct(_vector, 1.0f);

	moveVector = Vector3::Add(moveVector, knockVector);
}