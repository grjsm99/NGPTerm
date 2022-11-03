#pragma once

#include "Shader.h"
#include "Material.h"

class GameObject;

class Mesh {
protected:
	static shared_ptr<Shader> shader;
	static shared_ptr<Shader> instanceShader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
	static shared_ptr<Shader> GetInstanceShader();
protected:
	string name;


	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	UINT nVertex;	// ���ؽ�(������ ��ֺ���)�� ����

	ComPtr<ID3D12Resource> pPositionBuffer;	// ���ؽ��� ��ġ ����
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pNormalBuffer;		// ��ֺ����� ����
	ComPtr<ID3D12Resource> pNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW normalBufferView;

	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// �ؽ�ó ��ǥ�� ����
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

	vector<UINT> nSubMeshIndex;	// subMesh���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexBuffers;	// subMesh���� �ε��� ����
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW> subMeshIndexBufferViews;

	BoundingOrientedBox oobb;

public:		// �������� ��� �Լ���
	// ������ �� �Ҹ���
	Mesh();
	virtual ~Mesh();

public:		// ��� �Լ���

	// get, set�Լ�
	const string& GetName() const;
	const BoundingOrientedBox& GetOOBB() const;
	void LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj);
	void ReleaseUploadBuffers();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex);
	virtual void RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex, const D3D12_VERTEX_BUFFER_VIEW& _instanceBufferView, int _numInstance);
};

// ������� ��� �ϴ� �޽�

// boundingBox �޽�
class HitBoxMesh {
private:
	static shared_ptr<Shader> shader;
	
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	ComPtr<ID3D12Resource> pPositionBuffer;	// ���ؽ��� ��ġ ����
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pIndexBuffer;	// �ε��� ����
	ComPtr<ID3D12Resource> pIndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
public:
	HitBoxMesh();
	~HitBoxMesh();
	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};


class TerrainMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
protected:

	int width;
	int length;
	XMFLOAT3 scale;

	// OnGetColor���� �븻������ ���� �̸� ���. �븻��� ������ ������ ( �ѹ��� ����ϸ� �ǹǷ� )

	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	UINT nVertex;	// ���ؽ�(������ ��ֺ���)�� ����
	UINT nIndex;	// �ε����� ����

	ComPtr<ID3D12Resource> pPositionBuffer;	// ���ؽ��� ��ġ ����
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pColorBuffer;		// ��ֺ����� ����
	ComPtr<ID3D12Resource> pColorUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW colorBufferView;

	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// uv0�� ����
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

	ComPtr<ID3D12Resource> pTexCoord1Buffer;	// uv1�� ����
	ComPtr<ID3D12Resource> pTexCoord1UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord1BufferView;

	ComPtr<ID3D12Resource> pIndexBuffer;	// �ε��� ����
	ComPtr<ID3D12Resource> pIndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
public:
	TerrainMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _xStart, int _zStart, int _width, int _length, XMFLOAT3 _scale, XMFLOAT4 _color, void* _pContext);
	~TerrainMesh();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	float OnGetHeight(int x, int z, void* pContext);
	XMFLOAT4 OnGetColor(int x, int z, void* pContext);

};

class WaterMesh : public Mesh {
	
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();

public:
	WaterMesh(XMFLOAT3 _scale, float _height, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~WaterMesh();
};


class BillBoardMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();

private:
	int nBillboard;
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;

	ComPtr<ID3D12Resource> pPositionBuffer;	// �������� ��ġ ����
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pSizeBuffer;		// �������� ũ�� ����
	ComPtr<ID3D12Resource> pSizeUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW sizeBufferView;

	void* pContext;
public:
	BillBoardMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	BillBoardMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~BillBoardMesh();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class SkyBoxMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	
	ComPtr<ID3D12Resource> pPositionBuffer;
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;


	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// �ؽ�ó ��ǥ�� ����
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

public:
	SkyBoxMesh(int _id, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~SkyBoxMesh();
	void MakeResource(int _id, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};


/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
class MeshManager {
public:
	MeshManager();
	~MeshManager();

protected:
	unordered_map<string, shared_ptr<Mesh>> storage;
	HitBoxMesh hitBoxMesh;
public:
	HitBoxMesh& GetHitBoxMesh();
	shared_ptr<Mesh> GetMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj);
	void ReleaseUploadBuffers();
};
