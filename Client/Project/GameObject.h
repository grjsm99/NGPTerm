#pragma once
#include "Mesh.h"
#include "Status.h"

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05


class Light;
class Player;
class TerrainMap;
class Camera;

// �׽�Ʈ ��
static int guid = 0;

struct INSTANCING_FORMAT {
	XMFLOAT4X4 transform;
};

class GameObject : public enable_shared_from_this<GameObject> {
private:
	static ComPtr<ID3D12Resource> pInstanceBuffer;
	static ComPtr<ID3D12Resource>  pInstanceUploadBuffer;
	static D3D12_VERTEX_BUFFER_VIEW instanceBufferView;
	static INSTANCING_FORMAT* pMappedInstance;
public:
	static void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	static const D3D12_VERTEX_BUFFER_VIEW& GetInstanceBufferView();
	static INSTANCING_FORMAT* GetMappedData();
	
protected:
	int gid;
	string name;

	XMFLOAT4X4 worldTransform;
	
	// �θ���ǥ�� ����
	XMFLOAT4X4 localTransform;
	XMFLOAT3 localPosition;
	XMFLOAT3 localScale;
	XMFLOAT4 localRotation;

	// ��ü�� ������ �ִ� ���� ������
	shared_ptr<Light> pLight;


	BoundingOrientedBox boundingBox;

	// true�ϰ�� ���� ������Ʈ���� ��� �����ϴ� �ٿ���ڽ��� ����
	bool isOOBBCover;
	BoundingOrientedBox baseOrientedBox;
	
	
	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;
		
	weak_ptr<Mesh> pMesh;

	
	vector<shared_ptr<Material>> materials;

public:
	friend RigidBody;
	
	GameObject();
	virtual ~GameObject();
	// ���� ������Ʈ ���� ������
	//
	virtual void Create();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	// get set �Լ�
	const string& GetName() const;
	// �θ���ǥ����� ���͵��� ��´�.
	XMFLOAT3 GetLocalRightVector() const;
	XMFLOAT3 GetLocalUpVector() const;
	XMFLOAT3 GetLocalLookVector() const;
	XMFLOAT4 GetLocalRotate() const;
	XMFLOAT3 GetLocalPosition() const;
	// ���� �̵�
	void MoveRight(float distance);
	void Move(const XMFLOAT3& _moveVector, float _timeElapsed = 1.0f);
	void MoveUp(float distance);
	void MoveFront(float distance);

	void Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed = 1.0f);
	void Rotate(const XMFLOAT4& _quat);
	// ������ǥ�� ���� �ڽ��� ��ġ�� �����Ѵ�.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;

	// �ڽ��� �ٿ�� �ڽ��� ���۷����� �����Ѵ�.
	const BoundingOrientedBox& GetBoundingBox() const;

	// �ش� �ν��Ͻ��� ���� ������Ʈ�� ����
	shared_ptr<GameObject> GetObj();
	// ��ġ�� ������ �̵���Ų��.
	void SetLocalPosition(const XMFLOAT3& _position);
	// Ư�� ȸ������ �����Ѵ�.
	void SetLocalRotation(const XMFLOAT4& _rotation);
	// Ư�� Scale���� �����Ѵ�.
	void SetLocalScale(const XMFLOAT3& _scale);
	// �ش� ������Ʈ �������� �ֻ��� �θ� ( �ٿ���ڽ� Ŀ�� )�� ����
	void SetOOBBCover(bool _isCover);


	// �ڽ��� �߰��Ѵ�.
	void SetChild(const shared_ptr<GameObject> _pChild);
	// �޽��� �����Ѵ�.
	void SetMesh(const shared_ptr<Mesh>& _pMesh);	
	// �޽��� �ٿ�� �ڽ��� ������Ʈ�� �ű��.
	void SetBoundingBox(const BoundingOrientedBox& _box);

	void UpdateLocalTransform();
	// eachTransform�� ������ worldTransform�� ������Ʈ �Ѵ�.
	virtual void UpdateWorldTransform();
	// ��ȯ����� �����ϰ� worldTransform�� ������Ʈ �Ѵ�.

	//  OOBB ����
	void UpdateOOBB();
	void MergeOOBB(const shared_ptr<GameObject>& _coverObject);

	// ������Ʈ ���� ��ü������ ����
	void UpdateObject();

	// �浹 üũ
	void CheckCollision(const shared_ptr<GameObject>& _other);
	//bool CheckCollisionWithTerrain(shared_ptr<TerrainMap> _pTerrain, XMFLOAT3 _moveVector);
	// �ִϸ��̼�
	shared_ptr<GameObject> FindFrame(const string& _name);
	virtual void PrepareAnimate();
	virtual void Animate(double _timeElapsed);
	virtual void Animate(double _timeElapsed, const XMFLOAT3& _playerPos);
	virtual void Remove();
	virtual bool CheckRemove() const;
	

	// ����
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	void RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, list<shared_ptr<GameObject>>& _objList, const D3D12_VERTEX_BUFFER_VIEW& _instanceBV);
	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox);
	// ���� ��ȯ����� ���̴��� �Ѱ��ش�.
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariableInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _index);
	void UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);
	virtual void CopyObject(const GameObject& _other);
	
};

class GunShipObject : public GameObject {

public:
	GunShipObject();
	virtual ~GunShipObject();
	shared_ptr<GameObject> rotorFrame;
	shared_ptr<GameObject> tailRotorFrame;
	shared_ptr<GameObject> missileFrame;
	
public:
	virtual void PrepareAnimate();
	virtual void Animate(double _timeElapsed);
	virtual void CopyObject(const GameObject& _other);
};

class ApacheObject : public GameObject {
public:
	ApacheObject();
	virtual ~ApacheObject();
	shared_ptr<GameObject> rotorFrame;
	shared_ptr<GameObject> tailRotor1Frame;
	shared_ptr<GameObject> tailRotor2Frame;

public:
	virtual void PrepareAnimate();
	virtual void Animate(double _timeElapsed);
	virtual void CopyObject(const GameObject& _other);
};

class Missile : public GameObject, public RigidBody {
private:
	float lifeTime;
	UINT missileId;
	USHORT clientId;

public:
	Missile();
	virtual ~Missile();

public:
	void Create(USHORT _cid, UINT _mid, XMFLOAT3 _position, XMFLOAT4 _rotate, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	bool CheckRemove() const;
	virtual void Animate(double _timeElapsed);
	virtual void CopyObject(const GameObject& _other);
	void Remove();
};

class HeightMapImage {
private:
	BYTE* pHeightMapPixels;

	int							width;
	int							length;
	XMFLOAT3					scale;
	
public:
	HeightMapImage(LPCTSTR _fileName, int _width, int _length, XMFLOAT3 _scale);
	~HeightMapImage();
	float GetHeight(float x, float z, bool bReverseQuad = false);
	float GetWidth() { return (float)width; };
	float GetLength () { return (float)length; };
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(scale); }

	BYTE* GetHeightMapPixels() { return(pHeightMapPixels); }
	int GetHeightMapWidth() { return(width); }
	int GetHeightMapLength() { return(length); }
};



class TerrainMap {

private:
	XMFLOAT4X4 worldTransform;

	shared_ptr<HeightMapImage> pHeightMap;
	shared_ptr<Texture> pTexture;
	
	vector<shared_ptr<TerrainMesh>> pTerrainMeshs;
	
	int width;
	int length;
	
	XMFLOAT3 scale;
	
public:
	
	TerrainMap(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, LPCTSTR _fileName, int _width, int _length, int _blockWidth, int _blockLength, XMFLOAT3 _scale, XMFLOAT4 _color);
	~TerrainMap();
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(pHeightMap->GetHeight(x, z, bReverseQuad) * scale.y); };//World
	float GetWidth() { return pHeightMap->GetWidth(); };
	float GetLength() { return pHeightMap->GetLength(); };
	
	void SetMesh(int _index, shared_ptr<TerrainMesh> _pMesh) { pTerrainMeshs[_index] = _pMesh; };
	void SetTexture(shared_ptr<Texture> _pTexture) { pTexture = _pTexture; };
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	//void CreateShaderVariables(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	//void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE _cbvGPUDescriptorHandle) { cbvGPUDescriptorStartHandle = _cbvGPUDescriptorHandle; };
};

class Water : public GameObject {
private:
	// ref count ������
	shared_ptr<Mesh> ptMesh;

public:
	Water(XMFLOAT3 _size, float _height, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Animate(double _timeElapsed);
	~Water();
};

class BillBoard {
	
private:
	string name;
	
	unique_ptr<BillBoardMesh> pMesh;
	shared_ptr<Texture> pTexture;
public:
	BillBoard();
	~BillBoard();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Create(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class SkyBox {

	array<shared_ptr<SkyBoxMesh>, 6> pMeshs;
	array<shared_ptr<Texture>, 6> pTextures;
public:
	SkyBox();
	~SkyBox();
	
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera);
	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera);
};

class Effect {
private:
	// ������ ���̴��� ���
	static shared_ptr<BillBoardMesh> pMesh;
	static unordered_map<string, shared_ptr<Texture>> pTextures;
public:
	static void CreateBaseMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	static void LoadEffect(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
private:
	// ���� ���������� �Ѿ�� �ɸ��� �ð�
	string name;
	float nextFrameTime;
	// ��������Ʈ�� x,y�� �ִ� �������� ��
	USHORT xframeCount;
	USHORT yframeCount;
	
	float xsize, ysize;
	XMFLOAT2 startUV;
	XMFLOAT2 endUV;
	// �����ð� = nextFrameTime * x * y
	float life;
	
	float lifeTime;
	XMFLOAT3 position;
	
public:
	Effect();
	~Effect();
	void Create(float _nextFrameTime, USHORT _x, USHORT _y, float _xs, float _ys, const XMFLOAT3& _pos, const string& _name);
	
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Animate(float _timeElapsed);
	
	bool CheckRemove() const;
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};





class GameObjectManager {
	unordered_map<string, shared_ptr<GameObject>> storage;

public:
	shared_ptr<GameObject> GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
};

