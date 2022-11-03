#pragma once

class GameObject;
class TerrainMap;
// RigidBody ��� ��ü�� private�� �ϸ� �ǹǷ� struct�� ��
class RigidBody {
public:
	// �浹üũ�� ���� ������
	static shared_ptr<TerrainMap> pTerrain;
protected:
	// m/s
	XMFLOAT3 acc; // right, up, look
	float moveSpeed;
	XMFLOAT3 moveVector;
	XMFLOAT3 rotateAxis;

	XMFLOAT4X4 prevWorld;


	// degree/s
	float rotateAngle;

	weak_ptr<GameObject> self;
public:
	RigidBody();
	~RigidBody();

	void InitVector();
	void Jump(float _power);

	bool CheckCollisionWithTerrain(const shared_ptr<GameObject>& _obj);
	
	void RotateRigid(const XMFLOAT3& _axis, float _angle, float _timeElapsed);
	void MoveFrontRigid(bool _isfront, float _timeElapsed);
	void MoveRightRigid(bool _isright, float _timeElapsed);
	void MoveUpRigid(bool _isup, float _timeElapsed);
	
	void KnockBack(const XMFLOAT3& _vector);
};
