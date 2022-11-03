#include "stdafx.h"
#include "Mesh.h"
#include "GameFramework.h"

// ���� ���� �� �Լ�
shared_ptr<Shader> Mesh::shader;
shared_ptr<Shader> Mesh::instanceShader;
shared_ptr<Shader> HitBoxMesh::shader;
shared_ptr<Shader> TerrainMesh::shader;
shared_ptr<Shader> BillBoardMesh::shader;
shared_ptr<Shader> SkyBoxMesh::shader;
shared_ptr<Shader> WaterMesh::shader;

void Mesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {


	shader = make_shared<BasicShader>(_pDevice, _pRootSignature);
	instanceShader = make_shared<InstanceShader>(_pDevice, _pRootSignature);
	
}
shared_ptr<Shader> Mesh::GetShader() {
	return shader;
}

shared_ptr<Shader> Mesh::GetInstanceShader() {
	return instanceShader;
}

// ������, �Ҹ���
Mesh::Mesh() {
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}
Mesh::~Mesh() {

}

const string& Mesh::GetName() const {
	return name;
}

const BoundingOrientedBox& Mesh::GetOOBB() const {
	return oobb;
}

void Mesh::LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj) {
	GameFramework& gameFramework = GameFramework::Instance();
	ifstream file("Mesh/" + _fileName, ios::binary);	// ������ ����
	if (!file) {
		cout << "Mesh Load Failed : " << _fileName << "\n";
		return;
	}
	// ���ؽ��� ���� �б�
	file.read((char*)&nVertex, sizeof(UINT));
	// �� �̸� �ϱ�
	ReadStringBinary(name, file);
	
	// OOBB���� �б�
	XMFLOAT3 oobbCenter, oobbExtents;
	file.read((char*)&oobbCenter, sizeof(XMFLOAT3));
	file.read((char*)&oobbExtents, sizeof(XMFLOAT3));
	oobb = BoundingOrientedBox(oobbCenter, oobbExtents, XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f));
	_obj->SetBoundingBox(oobb);
	// �����ǰ� �б�
	vector<XMFLOAT3> positions(nVertex);

	file.read((char*)positions.data(), sizeof(float) * 3 * nVertex);
	// positions�� ���ҽ��� ����� ����
	
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;
	
	// ��ְ� �б�
	vector<float> normals(3 * nVertex);
	file.read((char*)normals.data(), sizeof(float) * 3 * nVertex);
	// normals�� ���ҽ��� ����� ����
	pNormalBuffer = CreateBufferResource(_pDevice, _pCommandList, normals.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pNormalUploadBuffer);
	normalBufferView.BufferLocation = pNormalBuffer->GetGPUVirtualAddress();
	normalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	normalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;
	

	// �ؽ�ó��ǥ�� �б�
	int nTexCoord = 0;
	file.read((char*)&nTexCoord, sizeof(int));

	vector<float> texcoords(2 * nTexCoord);
	file.read((char*)texcoords.data(), sizeof(float) * 2 * nTexCoord);
	
	// texture uv�� ���ҽ��� ����� ����
	pTexCoord0Buffer = CreateBufferResource(_pDevice, _pCommandList, texcoords.data(), sizeof(float) * 2 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoord0UploadBuffer);
	texCoord0BufferView.BufferLocation = pTexCoord0Buffer->GetGPUVirtualAddress();
	texCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	texCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * nVertex;
	
	// ����޽� ���� �б�
	UINT nSubMesh;
	file.read((char*)&nSubMesh, sizeof(UINT));
	// ����޽��� ������ŭ ���͸� �̸� �Ҵ��� ���´�.

	nSubMeshIndex.assign(nSubMesh, 0);
	pSubMeshIndexBuffers.assign(nSubMesh, {});
	pSubMeshIndexUploadBuffers.assign(nSubMesh, {});
	subMeshIndexBufferViews.assign(nSubMesh, {});
	
	for (UINT i = 0; i < nSubMesh; ++i) {
		file.read((char*)&nSubMeshIndex[i], sizeof(UINT));
		vector<UINT> indices(nSubMeshIndex[i]);
		file.read((char*)indices.data(), sizeof(UINT) * nSubMeshIndex[i]); 
		
		// subMeshIndices�� ���ҽ��� ����� ����
		pSubMeshIndexBuffers[i] = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * nSubMeshIndex[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pSubMeshIndexUploadBuffers[i]);
		subMeshIndexBufferViews[i].BufferLocation = pSubMeshIndexBuffers[i]->GetGPUVirtualAddress();
		subMeshIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		subMeshIndexBufferViews[i].SizeInBytes = sizeof(UINT) * nSubMeshIndex[i];
	}
}

void Mesh::ReleaseUploadBuffers() {
	if (pPositionUploadBuffer) pPositionUploadBuffer->Release();
	if (pNormalUploadBuffer) pNormalUploadBuffer->Release();
	if(pTexCoord0UploadBuffer) pTexCoord0UploadBuffer->Release();
	for (auto& pSubMeshIndexUploadBuffer : pSubMeshIndexUploadBuffers) {
		pSubMeshIndexUploadBuffer->Release();
	}
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex) {

	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[3] = { positionBufferView , normalBufferView, texCoord0BufferView };
	_pCommandList->IASetVertexBuffers(0, 3, vertexBuffersViews);

	
	_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[_subMeshIndex]);
	_pCommandList->DrawIndexedInstanced(nSubMeshIndex[_subMeshIndex], 1, 0, 0, 0);	
}

void Mesh::RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex, const D3D12_VERTEX_BUFFER_VIEW& _instanceBufferView, int _numInstance) {

	_pCommandList->IASetPrimitiveTopology(primitiveTopology);

	// ���ڷ� ���� �ν��Ͻ� ������ �̿��Ͽ� ������ŭ �Ѳ����� �׸���.
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[4] = { positionBufferView , normalBufferView, texCoord0BufferView, _instanceBufferView };
	_pCommandList->IASetVertexBuffers(0, 4, vertexBuffersViews);

	_pCommandList->IASetIndexBuffer(&subMeshIndexBufferViews[_subMeshIndex]);
	_pCommandList->DrawIndexedInstanced(nSubMeshIndex[_subMeshIndex], _numInstance, 0, 0, 0);
}

//////////////// HitBoxMesh ///////////////////

void HitBoxMesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	shader = make_shared<HitBoxShader>(_pDevice, _pRootSignature);
}

shared_ptr<Shader> HitBoxMesh::GetShader() {
	return shader;
}

HitBoxMesh::HitBoxMesh() {

}

HitBoxMesh::~HitBoxMesh() {

}

void HitBoxMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[1] = { positionBufferView };
	_pCommandList->IASetVertexBuffers(0, 1, vertexBuffersViews);
	_pCommandList->IASetIndexBuffer(&indexBufferView);
	_pCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0);
}

void HitBoxMesh::Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	XMFLOAT3 c(0,0,0);
	XMFLOAT3 e(0.5, 0.5, 0.5);
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	vector<XMFLOAT3> positions{
		XMFLOAT3(c.x - e.x, c.y - e.y, c.z - e.z),
		XMFLOAT3(c.x - e.x, c.y - e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y - e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y - e.y, c.z - e.z),
		XMFLOAT3(c.x - e.x, c.y + e.y, c.z - e.z),
		XMFLOAT3(c.x - e.x, c.y + e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y + e.y, c.z + e.z),
		XMFLOAT3(c.x + e.x, c.y + e.y, c.z - e.z),
	};

	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(XMFLOAT3) * 8, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * 8;

	vector<UINT> indices{
		0,1,1,2,
		2,3,3,0,
		4,5,5,6,
		6,7,7,4,
		0,4,1,5,
		2,6,3,7
	};

	pIndexBuffer = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * 24, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pIndexUploadBuffer);
	indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(UINT) * 24;

}

//////////////// TerrainMesh ////////////////////

TerrainMesh::TerrainMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _xStart, int _zStart, int _width, int _length, XMFLOAT3 _scale, XMFLOAT4 _color, void* _pContext) {
	nVertex = _width * _length;
	
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	
	width = _width;
	length = _length;
	scale = _scale;

	vector<XMFLOAT3> positions(nVertex);
	vector<XMFLOAT4> colors(nVertex);
	vector<XMFLOAT2> texcoord0(nVertex);
	vector<XMFLOAT2> texcoord1(nVertex);
	
	HeightMapImage* pHeightMapImage = (HeightMapImage*)_pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = _zStart; z < (_zStart + _length); z++)
	{
		for (int x = _xStart; x < (_xStart + _width); x++, i++)
		{
			XMFLOAT4 getColor = OnGetColor(x, z, _pContext);
			fHeight = OnGetHeight(x, z, _pContext);
			positions[i] = XMFLOAT3((x * _scale.x), fHeight, (z * _scale.z));
			colors[i] = Vector4::Add(getColor, _color);
			texcoord0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			texcoord1[i] = XMFLOAT2(float(x) / float(_scale.x * 0.5f), float(z) / float(_scale.z * 0.5f));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;

			
		}
	}
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(float) * 3;
	positionBufferView.SizeInBytes = sizeof(float) * 3 * nVertex;

	pColorBuffer = CreateBufferResource(_pDevice, _pCommandList, colors.data(), sizeof(float) * 4 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pColorUploadBuffer);
	colorBufferView.BufferLocation = pColorBuffer->GetGPUVirtualAddress();
	colorBufferView.StrideInBytes = sizeof(float) * 4;
	colorBufferView.SizeInBytes = sizeof(float) * 4 * nVertex;
	
	pTexCoord0Buffer = CreateBufferResource(_pDevice, _pCommandList, texcoord0.data(), sizeof(float) * 2 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoord0UploadBuffer);
	texCoord0BufferView.BufferLocation = pTexCoord0Buffer->GetGPUVirtualAddress();
	texCoord0BufferView.StrideInBytes = sizeof(float) * 2;
	texCoord0BufferView.SizeInBytes = sizeof(float) * 2 * nVertex;
	
	pTexCoord1Buffer = CreateBufferResource(_pDevice, _pCommandList, texcoord1.data(), sizeof(float) * 2 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoord1UploadBuffer);
	texCoord1BufferView.BufferLocation = pTexCoord1Buffer->GetGPUVirtualAddress();
	texCoord1BufferView.StrideInBytes = sizeof(float) * 2;
	texCoord1BufferView.SizeInBytes = sizeof(float) * 2 * nVertex;
	

	nIndex = ((_width * 2) * (_length - 1)) + ((_length - 1) - 1);
	vector<UINT> indices(nIndex);

	for (int j = 0, z = 0; z < _length - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < _width; x++)
			{
				if ((x == 0) && (z > 0)) indices[j++] = (UINT)(x + (z * _width));
				indices[j++] = (UINT)(x + (z * _width));
				indices[j++] = (UINT)((x + (z * _width)) + _width);
			}
		}
		else
		{
			for (int x = _width - 1; x >= 0; x--)
			{
				if (x == (_width - 1)) indices[j++] = (UINT)(x + (z * _width));
				indices[j++] = (UINT)(x + (z * _width));
				indices[j++] = (UINT)((x + (z * _width)) + _width);
			}
		}
	}
	pIndexBuffer = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * nIndex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pIndexUploadBuffer);
	indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(UINT) * nIndex;

}

TerrainMesh::~TerrainMesh() {
	
}

void TerrainMesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	shader = make_shared<TerrainShader>(_pDevice, _pRootSignature);
	

}

shared_ptr<Shader> TerrainMesh::GetShader() {
	return shader;
}


void TerrainMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {	
	// pos, color, texcoord0, texcoord1 ���۸� �Է������⿡ Set
	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[4] = { positionBufferView , colorBufferView, texCoord0BufferView, texCoord1BufferView };
	
	_pCommandList->IASetVertexBuffers(0, 4, vertexBuffersViews);
	_pCommandList->IASetIndexBuffer(&indexBufferView);
	_pCommandList->DrawIndexedInstanced(nIndex, 1, 0, 0, 0);
}


float TerrainMesh::OnGetHeight(int x, int z, void* pContext)
{
	HeightMapImage* pHeightMapImage = (HeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();

	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;

	return(fHeight);
}

XMFLOAT4 TerrainMesh::OnGetColor(int x, int z, void* pContext)
{
	// ������ �� �׸����� �븻�� ���� ���������ؼ� return
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	HeightMapImage* pHeightMapImage = (HeightMapImage*)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);

	XMFLOAT3 hNormal[4] = {
		pHeightMapImage->GetHeightMapNormal(x, z),
		pHeightMapImage->GetHeightMapNormal(x + 1, z),
		pHeightMapImage->GetHeightMapNormal(x + 1, z + 1),
		pHeightMapImage->GetHeightMapNormal(x, z + 1)
	};
	float fScale = Vector3::DotProduct(hNormal[0], xmf3LightDirection);
	fScale += Vector3::DotProduct(hNormal[1], xmf3LightDirection);
	fScale += Vector3::DotProduct(hNormal[2], xmf3LightDirection);
	fScale += Vector3::DotProduct(hNormal[3], xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return(xmf4Color);
}

///////////////////

void WaterMesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	shader = make_shared<WaterShader>(_pDevice, _pRootSignature);
}
shared_ptr<Shader> WaterMesh::GetShader() {
	return shader;
}


WaterMesh::WaterMesh(XMFLOAT3 _scale, float _height, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	name = "Mesh_water";
	nVertex = 4;
	array<XMFLOAT3, 4> positions;
	array<XMFLOAT3, 4> normals;
	array<UINT, 6> indices;
	array<XMFLOAT2, 4> uvs;

	positions[0] = XMFLOAT3(0, _height,0);
	positions[1] = XMFLOAT3(0, _height, _scale.z);
	positions[2] = XMFLOAT3(_scale.x, _height,0);
	positions[3] = XMFLOAT3(_scale.x, _height, _scale.z);
	
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	normals[0] = XMFLOAT3(0, 1, 0);
	normals[1] = XMFLOAT3(0, 1, 0);
	normals[2] = XMFLOAT3(0, 1, 0);
	normals[3] = XMFLOAT3(0, 1, 0);
	
	pNormalBuffer = CreateBufferResource(_pDevice, _pCommandList, normals.data(), sizeof(float) * 3 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pNormalUploadBuffer);
	normalBufferView.BufferLocation = pNormalBuffer->GetGPUVirtualAddress();
	normalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	normalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nVertex;

	int meterPerTile = 32;
	// 8m�� Ÿ�� �ϳ� ���.
	uvs[0] = XMFLOAT2(0, 0);
	uvs[1] = XMFLOAT2(0, _scale.z / meterPerTile);
	uvs[2] = XMFLOAT2(_scale.x / meterPerTile, 0);
	uvs[3] = XMFLOAT2(_scale.x / meterPerTile, _scale.z / meterPerTile);
	
	// texture uv�� ���ҽ��� ����� ����
	pTexCoord0Buffer = CreateBufferResource(_pDevice, _pCommandList, uvs.data(), sizeof(float) * 2 * nVertex, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoord0UploadBuffer);
	texCoord0BufferView.BufferLocation = pTexCoord0Buffer->GetGPUVirtualAddress();
	texCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	texCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * nVertex;
	
	UINT nSubMesh = 1;

	nSubMeshIndex.assign(nSubMesh, 0);
	pSubMeshIndexBuffers.assign(nSubMesh, {});
	pSubMeshIndexUploadBuffers.assign(nSubMesh, {});
	subMeshIndexBufferViews.assign(nSubMesh, {});

	nSubMeshIndex[0] = 6;
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;
	

	pSubMeshIndexBuffers[0] = CreateBufferResource(_pDevice, _pCommandList, indices.data(), sizeof(UINT) * nSubMeshIndex[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, pSubMeshIndexUploadBuffers[0]);
	subMeshIndexBufferViews[0].BufferLocation = pSubMeshIndexBuffers[0]->GetGPUVirtualAddress();
	subMeshIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	subMeshIndexBufferViews[0].SizeInBytes = sizeof(UINT) * nSubMeshIndex[0];
}

WaterMesh::~WaterMesh() {
	
}




//////////////

BillBoardMesh::BillBoardMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	GameFramework gameFramework = GameFramework::Instance();
	shared_ptr<TerrainMap> pTerrain = reinterpret_cast<PlayScene*>(gameFramework.GetCurrentScene().get())->GetTerrain();
	nBillboard = 1;
	vector<XMFLOAT3> positions(nBillboard);
	XMFLOAT2 size;
	positions[0] = XMFLOAT3(0, 0, 0);
	size = XMFLOAT2(1, 1);
	
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nBillboard, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(float) * 3;
	positionBufferView.SizeInBytes = sizeof(float) * 3 * nBillboard;

	pSizeBuffer = CreateBufferResource(_pDevice, _pCommandList, &size, sizeof(float) * 2, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pSizeUploadBuffer);
	sizeBufferView.BufferLocation = pSizeBuffer->GetGPUVirtualAddress();
	sizeBufferView.StrideInBytes = sizeof(float) * 2;
	sizeBufferView.SizeInBytes = sizeof(float) * 2;
}


BillBoardMesh::BillBoardMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	GameFramework gameFramework = GameFramework::Instance();
	shared_ptr<TerrainMap> pTerrain =  reinterpret_cast<PlayScene*>(gameFramework.GetCurrentScene().get())->GetTerrain();
	
	vector<XMFLOAT3> positions;
	XMFLOAT2 size;
	
	int xsize = pTerrain->GetWidth() * 8;
	int zsize = pTerrain->GetLength() * 8;
	
	
	if (_name == "Flower02") {
		nBillboard = 10000;
		size = XMFLOAT2(10.0f, 10.0f);
	}
	else if (_name == "Tree02") {
		nBillboard = 1000;
		size = XMFLOAT2(20.0f, 50.0f);
	}
	else if (_name == "Tree03") {
		nBillboard = 100;
		size = XMFLOAT2(20.0f, 60.0f);
	}
	else if (_name == "Grass01") {
		nBillboard = 1000000;
		size = XMFLOAT2(3.0f, 6.0f);
	}
	else if (_name == "Flower01") {
		nBillboard = 10000;
		size = XMFLOAT2(10.0f, 10.0f);
	}
	positions.resize(nBillboard);

	
	for (int i = 0; i < nBillboard; ++i) {
		XMFLOAT3 ran = XMFLOAT3(0,100,0);
		while (ran.y < WATER_HEIGHT + size.y / 2) {
			ran.x = -1;
			ran.z = -1;
			while (ran.x <= 0) ran.x = (int)random(8, 8888) % xsize - 8;
			while (ran.z <= 0) ran.z = (int)random(8, 8888) % zsize - 8;
			ran.y = pTerrain->GetHeight(ran.x, ran.z) + size.y / 2;
			
		}
		positions[i] = XMFLOAT3(ran.x, ran.y, ran.z);

	}

	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, positions.data(), sizeof(float) * 3 * nBillboard, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(float) * 3;
	positionBufferView.SizeInBytes = sizeof(float) * 3 * nBillboard;

	pSizeBuffer = CreateBufferResource(_pDevice, _pCommandList, &size, sizeof(float) * 2, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pSizeUploadBuffer);
	sizeBufferView.BufferLocation = pSizeBuffer->GetGPUVirtualAddress();
	sizeBufferView.StrideInBytes = sizeof(float) * 2;
	sizeBufferView.SizeInBytes = sizeof(float) * 2;
	
}


BillBoardMesh::~BillBoardMesh() {
	
}

void BillBoardMesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	shader = make_shared<BillBoardShader>(_pDevice, _pRootSignature);
}

shared_ptr<Shader> BillBoardMesh::GetShader() {
	return shader;
}


void BillBoardMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// pos, color, texcoord0, texcoord1 ���۸� �Է������⿡ Set
	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBuffersViews[2] = { positionBufferView, sizeBufferView  };
	
	_pCommandList->IASetVertexBuffers(0, 2, vertexBuffersViews);
	// 1���� ����ؼ� ������ ������ŭ �׸���. 
	_pCommandList->DrawInstanced(3, nBillboard, 0, 0);
}

SkyBoxMesh::SkyBoxMesh(int _id, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	MakeResource(_id, _pDevice, _pCommandList);
	
	
	
}
SkyBoxMesh::~SkyBoxMesh() {
	
}

void SkyBoxMesh::MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	shader = make_shared<SkyBoxShader>(_pDevice, _pRootSignature);
}

shared_ptr<Shader> SkyBoxMesh::GetShader() {
	return shader;
}


void SkyBoxMesh::MakeResource(int _id, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	float sz = 20.0f;
	array<XMFLOAT3, 6> vertices;
	array<XMFLOAT2, 6> uvs;
	// pos
	{
		if (_id == 0)
		{
			vertices[0] = XMFLOAT3(-sz, -sz, -sz);
			vertices[1] = XMFLOAT3(+sz, +sz, -sz);
			vertices[2] = XMFLOAT3(-sz, +sz, -sz);
			vertices[3] = XMFLOAT3(+sz, +sz, -sz);
			vertices[4] = XMFLOAT3(-sz, -sz, -sz);
			vertices[5] = XMFLOAT3(+sz, -sz, -sz);
		}
		if (_id == 1) {
			vertices[0] = XMFLOAT3(-sz, -sz, +sz);
			vertices[1] = XMFLOAT3(+sz, +sz, +sz);
			vertices[2] = XMFLOAT3(+sz, -sz, +sz);
			vertices[3] = XMFLOAT3(+sz, +sz, +sz);
			vertices[4] = XMFLOAT3(-sz, -sz, +sz);
			vertices[5] = XMFLOAT3(-sz, +sz, +sz);
		}
		if (_id == 2) {
			vertices[0] = XMFLOAT3(-sz, +sz, +sz);
			vertices[1] = XMFLOAT3(-sz, +sz, -sz);
			vertices[2] = XMFLOAT3(+sz, +sz, -sz);
			vertices[3] = XMFLOAT3(+sz, +sz, -sz);
			vertices[4] = XMFLOAT3(+sz, +sz, +sz);
			vertices[5] = XMFLOAT3(-sz, +sz, +sz);
		}
		if (_id == 3) {
			vertices[0] = XMFLOAT3(-sz, -sz, +sz);
			vertices[1] = XMFLOAT3(+sz, -sz, +sz);
			vertices[2] = XMFLOAT3(+sz, -sz, -sz);
			vertices[3] = XMFLOAT3(+sz, -sz, -sz);
			vertices[4] = XMFLOAT3(-sz, -sz, -sz);
			vertices[5] = XMFLOAT3(-sz, -sz, +sz);
		}
		if (_id == 4) {
			vertices[0] = XMFLOAT3(-sz, -sz, +sz);
			vertices[1] = XMFLOAT3(-sz, -sz, -sz);
			vertices[2] = XMFLOAT3(-sz, +sz, -sz);
			vertices[3] = XMFLOAT3(-sz, +sz, -sz);
			vertices[4] = XMFLOAT3(-sz, +sz, +sz);
			vertices[5] = XMFLOAT3(-sz, -sz, +sz);
		}
		if(_id == 5) {
			vertices[0] = XMFLOAT3(+sz, -sz, +sz);
			vertices[1] = XMFLOAT3(+sz, +sz, +sz);
			vertices[2] = XMFLOAT3(+sz, +sz, -sz);
			vertices[3] = XMFLOAT3(+sz, +sz, -sz);
			vertices[4] = XMFLOAT3(+sz, -sz, -sz);
			vertices[5] = XMFLOAT3(+sz, -sz, +sz);
		}
	}

	// uv
	{
		if (_id == 0) {
			uvs[0] = XMFLOAT2(1.0f, 0.0f);
			uvs[1] = XMFLOAT2(0.0f, 1.0f);
			uvs[2] = XMFLOAT2(1.0f, 1.0f);
			uvs[3] = XMFLOAT2(0.0f, 1.0f);
			uvs[4] = XMFLOAT2(1.0f, 0.0f);
			uvs[5] = XMFLOAT2(0.0f, 0.0f);
		}
		if (_id == 1) {
			uvs[0] = XMFLOAT2(0.0f, 0.0f);
			uvs[1] = XMFLOAT2(1.0f, 1.0f);
			uvs[2] = XMFLOAT2(1.0f, 0.0f);
			uvs[3] = XMFLOAT2(1.0f, 1.0f);
			uvs[4] = XMFLOAT2(0.0f, 0.0f);
			uvs[5] = XMFLOAT2(0.0f, 1.0f);
		}
		if (_id == 2) {
			uvs[0] = XMFLOAT2(0.0f, 0.0f);
			uvs[1] = XMFLOAT2(0.0f, 1.0f);
			uvs[2] = XMFLOAT2(1.0f, 1.0f);
			uvs[3] = XMFLOAT2(1.0f, 1.0f);
			uvs[4] = XMFLOAT2(1.0f, 0.0f);
			uvs[5] = XMFLOAT2(0.0f, 0.0f);			
		}
		if (_id == 3) {
			uvs[0] = XMFLOAT2(0.0f, 1.0f);
			uvs[1] = XMFLOAT2(1.0f, 1.0f);
			uvs[2] = XMFLOAT2(1.0f, 0.0f);
			uvs[3] = XMFLOAT2(1.0f, 0.0f);
			uvs[4] = XMFLOAT2(0.0f, 0.0f);
			uvs[5] = XMFLOAT2(0.0f, 1.0f);
		}
		if (_id == 4) {
			uvs[0] = XMFLOAT2(1.0f, 0.0f);
			uvs[1] = XMFLOAT2(0.0f, 0.0f);
			uvs[2] = XMFLOAT2(0.0f, 1.0f);
			uvs[3] = XMFLOAT2(0.0f, 1.0f);
			uvs[4] = XMFLOAT2(1.0f, 1.0f);
			uvs[5] = XMFLOAT2(1.0f, 0.0f);
		}
		if (_id == 5) {
			uvs[0] = XMFLOAT2(0.0f, 0.0f);
			uvs[1] = XMFLOAT2(0.0f, 1.0f);
			uvs[2] = XMFLOAT2(1.0f, 1.0f);
			uvs[3] = XMFLOAT2(1.0f, 1.0f);
			uvs[4] = XMFLOAT2(1.0f, 0.0f);
			uvs[5] = XMFLOAT2(0.0f, 0.0f);
		}
	}

	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, vertices.data(), sizeof(XMFLOAT3) * 6, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * 6;

	pTexCoord0Buffer = CreateBufferResource(_pDevice, _pCommandList, uvs.data(), sizeof(XMFLOAT2) * 6, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoord0UploadBuffer);
	texCoord0BufferView.BufferLocation = pTexCoord0Buffer->GetGPUVirtualAddress();
	texCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	texCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * 6;
	
}

void SkyBoxMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	_pCommandList->IASetPrimitiveTopology(primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2] = { positionBufferView, texCoord0BufferView };
	_pCommandList->IASetVertexBuffers(0, 2, vertexBufferViews);

	_pCommandList->DrawInstanced(6, 1, 0, 0);
}


/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
MeshManager::MeshManager() {
	
}

MeshManager::~MeshManager() {

}

HitBoxMesh& MeshManager::GetHitBoxMesh() {
	return hitBoxMesh;
}

shared_ptr<Mesh> MeshManager::GetMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj) {
	if(!storage.contains(_name)) {	// ó�� �ҷ��� �޽��� ���
		shared_ptr<Mesh> newMesh = make_shared<Mesh>();
		
		newMesh->LoadFromFile(_name, _pDevice, _pCommandList, _obj);
		
		storage[_name] = newMesh;
	}
	return storage[_name];
}

void MeshManager::ReleaseUploadBuffers() {
	for (auto& pMesh : storage) {
		pMesh.second->ReleaseUploadBuffers();
	}
}