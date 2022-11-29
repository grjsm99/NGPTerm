#include "stdafx.h"
#include "GameObject.h"
#include "Light.h"
#include "GameFramework.h"


ComPtr<ID3D12Resource> GameObject::pInstanceBuffer;
ComPtr<ID3D12Resource>  GameObject::pInstanceUploadBuffer;
D3D12_VERTEX_BUFFER_VIEW  GameObject::instanceBufferView;

INSTANCING_FORMAT* GameObject::pMappedInstance;

shared_ptr<BillBoardMesh> Effect::pMesh;
unordered_map<string, shared_ptr<Texture>> Effect::pTextures;
//////////////////////////////////////////


void GameObject::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// �ν��Ͻ� ������ ��� ���ҽ��� ����� Map
	pInstanceBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, sizeof(INSTANCING_FORMAT) * MAX_INSTANCE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pInstanceUploadBuffer);
	instanceBufferView.BufferLocation = pInstanceBuffer->GetGPUVirtualAddress();
	instanceBufferView.StrideInBytes = sizeof(INSTANCING_FORMAT);
	instanceBufferView.SizeInBytes = sizeof(INSTANCING_FORMAT) * MAX_INSTANCE;

	//pInstanceBuffer->Map(0, NULL, (void**)&pMappedInstance);

}

const D3D12_VERTEX_BUFFER_VIEW& GameObject::GetInstanceBufferView() {
	return instanceBufferView;
}

INSTANCING_FORMAT* GameObject::GetMappedData() {
	return pMappedInstance;
}

GameObject::GameObject() {
	name = "unknown";
	worldTransform = Matrix4x4::Identity();
	localTransform = Matrix4x4::Identity();
	localPosition = XMFLOAT3(0, 0, 0);
	localRotation = XMFLOAT4(0, 0, 0, 1);
	localScale = XMFLOAT3(1,1,1);
	boundingBox = BoundingOrientedBox();
	isOOBBCover = false;
	gid = guid++;
}
GameObject::~GameObject() {

}


void GameObject::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	GameObject::Create();
	shared_ptr<GameObject> temp = gameFramework.GetGameObjectManager().GetGameObject(_ObjectName, _pDevice, _pCommandList);
	
	// �ν��Ͻ��� �ڽ����� �� ������Ʈ�� ������ ����
	if(temp) SetChild(temp);
}

void GameObject::Create() {

}

const string& GameObject::GetName() const {
	return name;
}

XMFLOAT3 GameObject::GetLocalRightVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(1, 0, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalUpVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 1, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalLookVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 0, 1);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalPosition() const {
	return localPosition;
}
XMFLOAT4 GameObject::GetLocalRotate() const {
	return localRotation;
}

void GameObject::MoveRight(float distance) {
	XMFLOAT3 moveVector = GetLocalRightVector();	// RightVector�� �����ͼ�
	moveVector = Vector3::Normalize(moveVector);	// �������ͷ� �ٲ���
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// �̵��Ÿ���ŭ �����ش�.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void GameObject::Move(const XMFLOAT3& _moveVector, float _timeElapsed) {
	localPosition = Vector3::Add(localPosition, Vector3::ScalarProduct(_moveVector, _timeElapsed));
}

void GameObject::MoveUp(float distance) {
	XMFLOAT3 moveVector = GetLocalUpVector();	// UpVector�� �����ͼ�
	moveVector = Vector3::Normalize(moveVector);	// �������ͷ� �ٲ���
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// �̵��Ÿ���ŭ �����ش�.
	localPosition = Vector3::Add(localPosition, moveVector);
}
void GameObject::MoveFront(float distance) {
	XMFLOAT3 moveVector = GetLocalLookVector();	// LookVector�� �����ͼ�
	moveVector = Vector3::Normalize(moveVector);	// �������ͷ� �ٲ���
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// �̵��Ÿ���ŭ �����ش�.
	localPosition = Vector3::Add(localPosition, moveVector);
}
void GameObject::Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed) {
	localRotation = Vector4::QuaternionMultiply(localRotation, Vector4::QuaternionRotation(_axis, _angle * _timeElapsed));
}

void GameObject::Rotate(const XMFLOAT4& _quat) {

	localRotation = Vector4::QuaternionMultiply(localRotation, _quat);
}

XMFLOAT3 GameObject::GetWorldRightVector() const {
	return Vector3::Normalize(worldTransform._11, worldTransform._12, worldTransform._13);
}
XMFLOAT3 GameObject::GetWorldUpVector() const {
	return Vector3::Normalize(worldTransform._21, worldTransform._22, worldTransform._23);
}
XMFLOAT3 GameObject::GetWorldLookVector() const {
	return Vector3::Normalize(worldTransform._31, worldTransform._32, worldTransform._33);
}
XMFLOAT3 GameObject::GetWorldPosition() const {
	return XMFLOAT3(worldTransform._41, worldTransform._42, worldTransform._43);
}

const BoundingOrientedBox& GameObject::GetBoundingBox() const {
	return boundingBox;
}

shared_ptr<GameObject> GameObject::GetObj() {
	if (!pParent.lock()) return pChildren[0];
	else {
		cout << "�޼��尡 ȣ��� ������Ʈ�� �ν��Ͻ��� �ƴմϴ�.\n";
		return shared_from_this();
	}
}
void GameObject::SetLocalPosition(const XMFLOAT3& _position) {
	localPosition = _position;
}

void GameObject::SetLocalRotation(const XMFLOAT4& _rotation) {
	localRotation = _rotation;
}

void GameObject::SetLocalScale(const XMFLOAT3& _scale) {
	localScale = _scale;
}

void GameObject::SetOOBBCover(bool _isCover) {
	isOOBBCover = _isCover;
}

void GameObject::SetChild(const shared_ptr<GameObject> _pChild) {
	// �Ծ��� ���̰�, �θ� ���� ��� �θ�� ���� ������Ų��.
	if (auto pPreParent = _pChild->pParent.lock()) {
		pPreParent->pChildren.erase(ranges::find(pPreParent->pChildren, _pChild));
	}

	// ���� �ڽ����� �Ծ�
	pChildren.push_back(_pChild);

	// �ڽ��� �θ� ���� ����
	_pChild->pParent = shared_from_this();
}



void GameObject::SetMesh(const shared_ptr<Mesh>& _pMesh) {
	pMesh = _pMesh;
}

void GameObject::SetBoundingBox(const BoundingOrientedBox& _box) {
	baseOrientedBox = _box;
}

void GameObject::UpdateLocalTransform() {
	localTransform = Matrix4x4::Identity();
	// S

	localTransform._11 = localScale.x;
	localTransform._22 = localScale.y;
	localTransform._33 = localScale.z;
	// SxR
	localTransform = Matrix4x4::Multiply(localTransform, Matrix4x4::RotateQuaternion(localRotation));
	// xT
	localTransform._41 = localPosition.x;
	localTransform._42 = localPosition.y;
	localTransform._43 = localPosition.z;
}

void GameObject::UpdateWorldTransform() {
	//UpdateLocalTransform();

	if (auto pParentLock = pParent.lock()) {	// �θ� ���� ���
		worldTransform = Matrix4x4::Multiply(localTransform, pParentLock->worldTransform);
	}
	else {	// �θ� ���� ���
		worldTransform = localTransform;
	}

	// �ڽĵ鵵 worldTransform�� ������Ʈ ��Ų��.
	for (auto& pChild : pChildren) {
		pChild->UpdateWorldTransform();
	}
}



void GameObject::UpdateOOBB() {
	
	if (isOOBBCover) {
		XMFLOAT3 tmp = baseOrientedBox.Center;
		baseOrientedBox.Transform(boundingBox, XMLoadFloat4x4(&worldTransform));

		XMStoreFloat4(&boundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&boundingBox.Orientation)));


	}
	for (const auto& pChild : pChildren) {
		pChild->UpdateOOBB();
	}
}

void GameObject::MergeOOBB(const shared_ptr<GameObject>& _coverObject) {
	if (!isOOBBCover) {

		BoundingOrientedBox& coverBox = _coverObject->baseOrientedBox;
		if (GetName() == "GameObject_Gunship") {
			// gunship�� ��� boundingBox�� �߽��� ������ �ƴ� 0 1.012005 -4.939685�̹Ƿ� ��������
			coverBox.Center = pMesh.lock()->GetOOBB().Center;
		}
		// �� OOBB�� �Ÿ����� ����
		XMFLOAT3 gapPosition = Vector3::Subtract(_coverObject->GetWorldPosition(), GetWorldPosition());

		XMFLOAT3 worldCenter = Vector3::Add(baseOrientedBox.Center, gapPosition);
		XMFLOAT3 distance = Vector3::Subtract(worldCenter, coverBox.Center);

		coverBox.Extents.x = max(abs(distance.x) + baseOrientedBox.Extents.x, coverBox.Extents.x);
		coverBox.Extents.y = max(abs(distance.y) + baseOrientedBox.Extents.y, coverBox.Extents.y);
		coverBox.Extents.z = max(abs(distance.z) + baseOrientedBox.Extents.z, coverBox.Extents.z);
	}
	for (const auto& pChild : pChildren) {
		pChild->MergeOOBB(_coverObject);
	}
}

void GameObject::UpdateObject() {
	UpdateLocalTransform();
	UpdateWorldTransform();
	UpdateOOBB();
}

void GameObject::CheckCollision(const shared_ptr<GameObject>& _other) {
	// �̻��
	if (isOOBBCover) {
		if (_other->isOOBBCover)
		{ 
			if (boundingBox.Intersects(_other->boundingBox)) {
				//CollideReact(shared_from_this(), _other);
			}
			return;
		}
	}

	// �Ѵ� Cover�϶����� ������
	for (const auto& pChild : _other->pChildren) {
		CheckCollision(pChild);
	}
	for (const auto& pChild : pChildren) {
		pChild->CheckCollision(_other);
	}
}

shared_ptr<GameObject> GameObject::FindFrame(const string& _name) {
	if (name == _name) {
		return shared_from_this();
	}
	for (const auto& pChild : pChildren) {
		if (auto pFound = pChild->FindFrame(_name)) {
			return pFound;
		}
	}
	return nullptr;
}

void GameObject::PrepareAnimate() {
	
}

void GameObject::Animate(double _timeElapsed) {
	for (const auto& pChild : pChildren) {
		pChild->Animate(_timeElapsed);
	}
}

void GameObject::Animate(double _timeElapsed, const XMFLOAT3& _playerPos) {

};
void GameObject::Remove() {
	
};

bool GameObject::CheckRemove() const {
	return false;
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	if (pMesh.lock()) {	// �޽��� ���� ��쿡�� �������� �Ѵ�.
		UpdateShaderVariable(_pCommandList);
		
		// �� ���׸��� �´� ����޽��� �׸���.
		for (int i = 0; i < materials.size(); ++i) {
			// �ش� ����Ž��� ��Ī�Ǵ� ���׸����� Set ���ش�.

			materials[i]->UpdateShaderVariable(_pCommandList); 
			pMesh.lock()->Render(_pCommandList, i);
		}
	}
	for (const auto& pChild : pChildren) {
		pChild->Render(_pCommandList);
	}
}

void GameObject::RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, list<shared_ptr<GameObject>>& _objList, const D3D12_VERTEX_BUFFER_VIEW& _instanceBV) {
	// �̻��
	
	// �� �����Ӹ��� �� �������� ���� ���庯ȯ ����� �������ش�.

	if (pMesh.lock()) {
		// UpdateShaderVariable�� ��������� �����ϴ°��� �ƴ�, ���ҽ��� ���� (IA)

		/*int i = 0;
		for (auto& pObj : _objList) {
			shared_ptr<GameObject> cur_frame = pObj->FindFrame(name);
			cout << cur_frame->GetName() << "\n";
			cur_frame->UpdateShaderVariableInstance(_pCommandList, i++);
		}*/
		
		// �� ����޽��� ���� DP call�� �ѹ��� ���ش�.
		for (int i = 0; i < materials.size(); ++i) {
			materials[i]->UpdateShaderVariable(_pCommandList);
			pMesh.lock()->RenderInstancing(_pCommandList, i, _instanceBV, _objList.size());
		}
	}
	for (const auto& pChild : pChildren) {
		pChild->RenderInstancing(_pCommandList, _objList, _instanceBV);
	}
}


void GameObject::RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox) {
	
	if (isOOBBCover) {	// �޽��� ���� ��쿡�� �������� �Ѵ�.
		UpdateHitboxShaderVariable(_pCommandList);
		// ����� ���̴��� �׷��Ƚ� ������������ �����Ѵ� [�������]
		_hitBox.Render(_pCommandList);
	}
	for (const auto& pChild : pChildren) {
		pChild->RenderHitBox(_pCommandList, _hitBox);
	}
}

void GameObject::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

void GameObject::UpdateShaderVariableInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _index) {
	INSTANCING_FORMAT* pInstancing = GameObject::GetMappedData();
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	memcpy(&pInstancing[_index].transform, &world, sizeof(INSTANCING_FORMAT));
}


void GameObject::UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	XMFLOAT4X4 world = Matrix4x4::ScaleTransform(Vector3::ScalarProduct(baseOrientedBox.Extents, 1.0f));
	XMFLOAT4X4 translate = Matrix4x4::Identity();
	translate._41 += baseOrientedBox.Center.x;
	translate._42 += baseOrientedBox.Center.y;
	translate._43 += baseOrientedBox.Center.z;
	// �ٿ���ڽ��� extents, center�� ��ŭ �̵� �� ��ȯ ��� ���� ( ���� )
	world = Matrix4x4::Multiply(Matrix4x4::Multiply(world, translate), worldTransform);

	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
	//world = Matrix4x4::Identity();
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
	
}



void GameObject::LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject) {
	GameFramework& gameFramework = GameFramework::Instance();

	// nameSize (UINT) / name(string)
	ReadStringBinary(name, _file);

	// localTransform(float3, 3, 4)
	_file.read((char*)&localPosition, sizeof(XMFLOAT3));
	_file.read((char*)&localScale, sizeof(XMFLOAT3));
	_file.read((char*)&localRotation, sizeof(XMFLOAT4));
	UpdateLocalTransform();

	string meshFileName;
	// meshNameSize(UINT) / meshName(string)
	ReadStringBinary(meshFileName, _file);
	
	// �޽ð� ������� ��ŵ
	if (meshFileName.size() != 0) {

		pMesh = gameFramework.GetMeshManager().GetMesh(meshFileName, _pDevice, _pCommandList, shared_from_this());
		if (pMesh.lock()->GetName().contains("body")) {
			_coverObject->SetBoundingBox(boundingBox);
		}
		// ���׸��� ����
		// nMaterial (UINT)
		UINT nMaterial = 0;
		_file.read((char*)&nMaterial, sizeof(UINT));
		materials.resize(nMaterial);

		// ���׸������ �ҷ���
		for (auto& mat : materials) {
			mat = make_shared<Material>();
			mat->LoadMaterial(_file, _pDevice, _pCommandList);
		}
	}
	int nChildren;
	_file.read((char*)&nChildren, sizeof(int));
	pChildren.reserve(nChildren);

	for (int i = 0; i < nChildren; ++i) {
		shared_ptr<GameObject> newObject = make_shared<GameObject>();
		newObject->LoadFromFile(_file, _pDevice, _pCommandList, _coverObject);
		SetChild(newObject);
	}
}

void GameObject::CopyObject(const GameObject& _other) {
	name = _other.name;
	worldTransform = _other.worldTransform;
	localTransform = _other.localTransform;
	boundingBox = _other.boundingBox;
	baseOrientedBox = _other.baseOrientedBox;
	isOOBBCover = _other.isOOBBCover;
	localPosition = _other.localPosition;
	localScale = _other.localScale;
	localRotation = _other.localRotation;
	pMesh = _other.pMesh;
	materials = _other.materials;

	for (int i = 0; i < _other.pChildren.size(); ++i) {
		shared_ptr<GameObject> child = make_shared<GameObject>();
		child->CopyObject(*_other.pChildren[i]);
		SetChild(child);
	}
}

////// GunshipObject , ApacheObject ////// 

GunShipObject::GunShipObject() {
	
}

GunShipObject::~GunShipObject() {

}

void GunShipObject::PrepareAnimate() {
	rotorFrame = FindFrame("GameObject_Rotor");
	tailRotorFrame = FindFrame("GameObject_Back_Rotor");
//	missileFrame = FindFrame("")
}

void GunShipObject::Animate(double _fTimeElapsed) {

	if (rotorFrame) {
		rotorFrame->Rotate(rotorFrame->GetLocalUpVector(), 1440.0f * (float)_fTimeElapsed);
		rotorFrame->UpdateObject();

	}
	if (tailRotorFrame) {
		tailRotorFrame->Rotate(tailRotorFrame->GetLocalRightVector(), 1440.0f * (float)_fTimeElapsed);
		tailRotorFrame->UpdateObject();
	}
}

void GunShipObject::CopyObject(const GameObject& _other) {
	GunShipObject& tmp = (GunShipObject&)_other;
	
	rotorFrame = tmp.rotorFrame;
	tailRotorFrame = tmp.tailRotorFrame;

	GameObject::CopyObject(_other);

	PrepareAnimate();
}


ApacheObject::ApacheObject() {

}

ApacheObject::~ApacheObject() {

}

void ApacheObject::PrepareAnimate() {
	rotorFrame = FindFrame("GameObject_rotor");
	tailRotor1Frame = FindFrame("GameObject_black m 6");
	tailRotor2Frame = FindFrame("GameObject_black m 7");
	//	missileFrame = FindFrame("")
}

void ApacheObject::Animate(double _fTimeElapsed) {
	if (rotorFrame) {
		rotorFrame->Rotate(rotorFrame->GetLocalUpVector(), 1440.0f * _fTimeElapsed);
		rotorFrame->UpdateObject();
	}
	if (tailRotor1Frame) {
		tailRotor1Frame->Rotate(XMFLOAT3(1,0,0), 1440.0f * _fTimeElapsed);
		tailRotor1Frame->UpdateObject();
	}
	if (tailRotor2Frame) {
		tailRotor2Frame->Rotate(XMFLOAT3(1, 0, 0), 1440.0f * _fTimeElapsed);
		tailRotor2Frame->UpdateObject();
	}
}

void ApacheObject::CopyObject(const GameObject& _other) {
	ApacheObject& tmp = (ApacheObject&)_other;

	rotorFrame = tmp.rotorFrame;
	tailRotor1Frame = tmp.tailRotor1Frame;

	GameObject::CopyObject(_other);

	PrepareAnimate();
}

////// MissileObject //////

Missile::Missile() {
	lifeTime = 2.0f;
}

Missile::~Missile() {
	
}
void Missile::Create(USHORT _cid, UINT _mid, XMFLOAT3 _position, XMFLOAT4 _rotate, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	clientId = _cid;
	missileId = _mid;
	name = "Missile";
	GameObject::Create("Missile", _pDevice, _pCommandList);
	self = shared_from_this();
	SetLocalScale(XMFLOAT3(3,3,3));
	
	//SetChild(gameFramework.GetGameObjectManager().GetGameObject(, _pDevice, _pCommandList));
	moveSpeed = 0.0f;
	XMFLOAT3 nPos = _position;

	SetLocalPosition(nPos);
	SetLocalRotation(_rotate);
	UpdateObject();
}

bool Missile::CheckRemove() const {
	return lifeTime <= 0.0f;
}

void Missile::Animate(double _timeElapsed) {
	lifeTime -= _timeElapsed;
	moveSpeed += 150.0f * _timeElapsed;
	MoveFrontRigid(true, _timeElapsed);

	if(!CheckCollisionWithTerrain(shared_from_this()))
	{
		Rotate(rotateAxis, rotateAngle, _timeElapsed);
		Move(moveVector, _timeElapsed);
	}
	if (moveVector.x != 0 || moveVector.y != 0 || moveVector.z != 0 || rotateAngle != 0.0f)
		UpdateObject();
	InitVector();
}

void Missile::CopyObject(const GameObject& _other) {
	Missile& tmp = (Missile&)_other;

	GameObject::CopyObject(_other);
	PrepareAnimate();
}

void Missile::Remove() {
	lifeTime = 0.0f;
}


//////////// HeightMapImage, TerrainMap ////////////


HeightMapImage::HeightMapImage(LPCTSTR _fileName, int _width, int _length, XMFLOAT3 _scale) {

	width = _width;
	length = _length;
	scale = _scale;
	BYTE* ptemp = new BYTE[width * length];

	ifstream in(_fileName, ios::binary);
	in.read((char*)ptemp, (width * length));

	pHeightMapPixels = new BYTE[width * length];
	for (int y = 0; y < length; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pHeightMapPixels[x + ((length - 1 - y) * width)] = ptemp[x + (y * width)];
		}
	}

	if (ptemp) delete[] ptemp;
}

HeightMapImage::~HeightMapImage() {
	if (pHeightMapPixels) delete[] pHeightMapPixels;
	pHeightMapPixels = NULL;
}


float HeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / scale.x;
	fz = fz / scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= width) || (fz >= length)) return(INT_MAX);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)pHeightMapPixels[x + (z * width)];
	float fBottomRight = (float)pHeightMapPixels[(x + 1) + (z * width)];
	float fTopLeft = (float)pHeightMapPixels[x + ((z + 1) * width)];
	float fTopRight = (float)pHeightMapPixels[(x + 1) + ((z + 1) * width)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}


XMFLOAT3 HeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= width) || (z >= length)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * width);
	int xHeightMapAdd = (x < (width - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (length - 1)) ? width : -width;
	float y1 = (float)pHeightMapPixels[nHeightMapIndex] * scale.y;
	float y2 = (float)pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * scale.y;
	float y3 = (float)pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

TerrainMap::TerrainMap(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, LPCTSTR _fileName, int _width, int _length, int _blockWidth, int _blockLength, XMFLOAT3 _scale, XMFLOAT4 _color) {
	
	worldTransform = Matrix4x4::Identity();
	GameFramework gameFramework = GameFramework::Instance();

	width = _width;
	length = _length;

	int cxQuadsPerBlock = _blockWidth - 1;
	int czQuadsPerBlock = _blockLength - 1;

	scale = _scale;

	pHeightMap = make_shared<HeightMapImage>(_fileName, width, length, scale);
	
	long cxBlocks = (width - 1) / cxQuadsPerBlock;
	long czBlocks = (length - 1) / czQuadsPerBlock;
	
	// ���� �޽��� �� ���� �Ҵ�
	pTerrainMeshs.assign(cxBlocks * czBlocks, {});
	
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (_blockWidth - 1);
			zStart = z * (_blockLength - 1);
			shared_ptr<TerrainMesh> terrainMesh = make_shared<TerrainMesh>(_pDevice, _pCommandList, xStart, zStart, _blockWidth, _blockLength, _scale, _color, pHeightMap.get());
			SetMesh(x + (z * cxBlocks), terrainMesh);

		}
	}


	// �ؽ�ó �ΰ�, ��Ʈ �Ķ���ʹ� 1���� ���
	shared_ptr< Texture> pTerrainTexture = make_shared<Texture>(2, RESOURCE_TEXTURE2D, 0, 1);
	
	//  �� ���̽�, ������ �ؽ�ó
	pTerrainTexture->LoadFromFile("Base_Texture", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 5);

	pTerrainTexture->LoadFromFile("Detail_Texture_7", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 1, 5);

	shared_ptr<Shader> pTerrainShader = TerrainMesh::GetShader();

	// �������̴��� SRV�� �����ؼ� �־���
	pTerrainShader->CreateShaderResourceViews(_pDevice, pTerrainTexture, 0, 5);

	SetTexture(pTerrainTexture);

}

TerrainMap::~TerrainMap() {

}


void TerrainMap::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

void TerrainMap::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pTerrainMeshs.size() > 0) {
		
		// ������ �⺻ ���� ��İ��� ������Ʈ
		UpdateShaderVariable(_pCommandList);
		// �ؽ�ó�� �ִٸ� ������Ʈ �Ѵ�.
		if (pTexture) {
				pTexture->UpdateShaderVariable(_pCommandList);
				// ���� �޽� ������ �������� 0���ε����� Render.
				pTerrainMeshs[0]->Render(_pCommandList);
		}
		
	}
}

/////////// Water ///////////////

Water::Water(XMFLOAT3 _size, float _height, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	name = "water";
	GameFramework& gameFramework = GameFramework::Instance();

	ptMesh = make_shared<WaterMesh>(_size, _height, _pDevice, _pCommandList);
	pMesh = ptMesh;

	shared_ptr<Texture> pTexture = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadFromFile("water", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);

	materials.resize(1);
	materials[0] = make_shared<Material>();
	materials[0]->DefaultMaterial(_pDevice, _pCommandList);
	materials[0]->SetTexture(pTexture);
	
	shared_ptr<Shader> pWaterShader = WaterMesh::GetShader();
	pWaterShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
}

void Water::Animate(double _timeElapsed) {
	worldTransform._41 += _timeElapsed * 0.2f;
	worldTransform._42 += _timeElapsed * 0.08f;
}

Water::~Water() {
	
}
//////////// BillBoard ////////////////

BillBoard::BillBoard() {

}

BillBoard::~BillBoard() {

}

void BillBoard::Create(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	pTexture = gameFramework.GetTextureManager().GetTexture(_name, _pDevice, _pCommandList);
	pMesh = make_unique<BillBoardMesh>(_name, _pDevice, _pCommandList);
	
	shared_ptr<Shader> pBillBoardShader = BillBoardMesh::GetShader();

	// ������ ���̴��� �� ���� �ؽ�ó SRV�� �����ؼ� �־���
	pBillBoardShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
}

void BillBoard::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	if (pMesh) {
		if (pTexture) {
			pTexture->UpdateShaderVariable(_pCommandList);
		}
		pMesh->Render(_pCommandList);
		
	}
}

///////////// SkyBox ////////////////

SkyBox::SkyBox() {

}

SkyBox::~SkyBox() {

}



//////////// Effect //////////

Effect::Effect() {
	endUV = XMFLOAT2(0, 0);
	startUV = XMFLOAT2(1, 1);
	life = 0;
	lifeTime = 0;
	position = XMFLOAT3(0, 0, 0);
	xframeCount = 0;
	yframeCount = 0;
	nextFrameTime = 0;
	xsize = 0;
	ysize = 0;
}

Effect::~Effect() {

}

void Effect::CreateBaseMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	pMesh = make_shared<BillBoardMesh>(_pDevice, _pCommandList);
}

void Effect::LoadEffect(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	shared_ptr<Texture> pTexture = gameFramework.GetTextureManager().GetTexture(_name, _pDevice, _pCommandList);
	pTextures[_name] = pTexture;
	
	shared_ptr<Shader> pBillBoardShader = BillBoardMesh::GetShader();
	pBillBoardShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
}

void Effect::Create(float _nextFrameTime, USHORT _x, USHORT _y, float _xs, float _ys, const XMFLOAT3& _pos, const string& _name) {
	name = _name;
	xframeCount = _x;
	yframeCount = _y;
	xsize = _xs;
	ysize = _ys;
	nextFrameTime = _nextFrameTime;
	lifeTime = nextFrameTime * xframeCount * yframeCount;
	life = lifeTime;
	position = _pos;
}

void Effect::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	UpdateShaderVariable(_pCommandList);
	if (pMesh) {
		if (pTextures[name]) {
			pTextures[name]->UpdateShaderVariable(_pCommandList);
		}
		pMesh->Render(_pCommandList);
	}
}

void Effect::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world = Matrix4x4::Identity();
	world._12 = startUV.x;
	world._13 = startUV.y;
	
	world._11 = endUV.x;
	world._22 = endUV.y;

	world._33 = xsize;
	world._44 = ysize;
	
	world._41 = position.x;
	world._42 = position.y;
	world._43 = position.z;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

void Effect::Animate(float _timeElapsed) {
	// lifeTime = life�϶��� 0������, lifeTime = 0�϶� xsize * ysize - 1������
	USHORT nowFrame = (USHORT)((life - lifeTime) / nextFrameTime);
	lifeTime -= _timeElapsed;
	startUV.x = (nowFrame % xframeCount) / (float)xframeCount;
	startUV.y = (nowFrame / yframeCount) / (float)yframeCount;
	endUV.x = startUV.x + 1.0f / xframeCount;
	endUV.y = startUV.y + 1.0f / yframeCount;
}

bool Effect::CheckRemove() const {
	return lifeTime < 0.0f;
}

void SkyBox::Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// ť�� ������Ʈ ���ҽ� ����
	for (int i = 0; i < 6; i++) {
		pMeshs[i] = make_shared<SkyBoxMesh>(i, _pDevice, _pCommandList);
		pTextures[i] = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	}
	// �յ�, ����, �޿�
	pTextures[0]->LoadFromFile("SkyBox_Back_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[1]->LoadFromFile("SkyBox_Front_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[2]->LoadFromFile("SkyBox_Top_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[3]->LoadFromFile("SkyBox_Bottom_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[4]->LoadFromFile("SkyBox_Left_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[5]->LoadFromFile("SkyBox_Right_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	//pTextures[5]->LoadFromFile("Flower02", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);

	//GameFramework gameFramework = gameFramework.Instance();
	
	shared_ptr<Shader> pSkyBoxShader = SkyBoxMesh::GetShader();
	for (int i = 0; i < 6; i++) {
		pSkyBoxShader->CreateShaderResourceViews(_pDevice, pTextures[i], 0, 4);
	}

	// ��ī�̹ڽ� ���̴��� �� ���� �ؽ�ó SRV�� �����ؼ� �־���
}

void SkyBox::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera) {
	
	// ī�޶� �������� worldTransform���� �����ش�.
	UpdateShaderVariable(_pCommandList, _pCamera);
	
	for (int i = 0; i < 6; ++i) {
		if (pTextures[i]) {
			pTextures[i]->UpdateShaderVariable(_pCommandList);
			if(pMeshs[i])
				pMeshs[i]->Render(_pCommandList);
		}
	}
}

void SkyBox::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera) {
	XMFLOAT4X4 world = Matrix4x4::Identity();
	XMFLOAT3 cameraPos = _pCamera->GetWorldPosition();
	world._41 = cameraPos.x;
	world._42 = cameraPos.y;
	world._43 = cameraPos.z;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

/////////////////////////// GameObjectManager /////////////////////
shared_ptr<GameObject> GameObjectManager::GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	if (!storage.contains(_name)) {	// ó�� �ҷ��� ������Ʈ�� ���
		shared_ptr<GameObject> newObject;
		if (_name == "Gunship") {
			newObject = make_shared<GunShipObject>();
		}
		else if (_name == "Apache") {
			newObject = make_shared<ApacheObject>();
		}
		else if (_name == "Missile") {
			newObject = make_shared<Missile>();
		}
		else  newObject  = make_shared<GameObject>();
		ifstream file("GameObject/" + _name, ios::binary);	// ������ ����
		
		if (!file) {
			cout << "GameObject Load Failed : " << _name << "\n";
			return nullptr;
		}
		
		newObject->SetOOBBCover(true);
		newObject->LoadFromFile(file, _pDevice, _pCommandList, newObject);

		BoundingOrientedBox box;
		file.read((char*)&box.Center, sizeof(XMFLOAT3));
		file.read((char*)&box.Extents, sizeof(XMFLOAT3));
		newObject->SetBoundingBox(box);
		// eachTransfrom�� �°� �� ������ ������Ʈ���� worldTransform�� ����
		storage[_name] = newObject;
		newObject->UpdateObject();
	}
	// ���丮�� �� ������Ʈ ������ ���� ������Ʈ�� �����Ͽ� �����Ѵ�.
	shared_ptr<GameObject> Object;
	if (_name == "Gunship") Object = make_shared<GunShipObject>();
	else if (_name == "Apache") Object = make_shared<ApacheObject>();
	else if (_name == "Missile") Object = make_shared<Missile>();

	else  Object = make_shared<GameObject>();
	
	Object->CopyObject(*storage[_name]);
	return Object;
}

