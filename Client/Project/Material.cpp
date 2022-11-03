#include "stdafx.h"
#include "Material.h"
#include "GameFramework.h"

Material::Material() {

}

Material::~Material() {
	pMaterialBuffer->Unmap(0, NULL);
}

void Material::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// �⺻������ ���� �ʴ´ٸ� ������ �ʴ´�.
	if (pMaterialBuffer) {
		D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pMaterialBuffer->GetGPUVirtualAddress();
		_pCommandList->SetGraphicsRootConstantBufferView(3, gpuVirtualAddress);
	}

	if (pTexture) {
		// ���⼭ �ؽ�ó ������ ���̴��� �ø�

		pTexture->UpdateShaderVariable(_pCommandList);
	}
}

void Material::LoadMaterial(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// ���׸��� ���� ���ҽ� ������ map

	GameFramework& gameFramework = GameFramework::Instance();
	
	string textureName;
	ReadStringBinary(textureName, _file);


	// �ش� �̸��� ���� �ؽ�ó�� ������
	if (textureName != "null") {
		pTexture = gameFramework.GetTextureManager().GetTexture(textureName, _pDevice, _pCommandList);
		if (!pTexture) {
			nType = 0;
		}
		else {
			nType = 1;
			auto pShader = Mesh::GetShader();
			pShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
		}
	}
	
	shared_ptr<VS_MaterialMappedFormat> pMappedMaterial;

	UINT cbElementSize = (sizeof(VS_MaterialMappedFormat) + 255) & (~255);
	ComPtr<ID3D12Resource> temp;
	pMaterialBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pMaterialBuffer->Map(0, NULL, (void**)&pMappedMaterial);
	
	XMFLOAT4 ambient, diffuse, specular, emissive;
	XMFLOAT4 mp = XMFLOAT4(0, 0, 0, 1);
	// format�� �´°��� ���Ͽ��� �о� ���ҽ��� ����
	_file.read((char*)&ambient, sizeof(XMFLOAT4));
	_file.read((char*)&diffuse, sizeof(XMFLOAT4));
	_file.read((char*)&specular, sizeof(XMFLOAT4));
	_file.read((char*)&emissive, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->ambient, &ambient, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->diffuse, &diffuse, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->specular, &specular, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->emissive, &emissive, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->nTypes, &nType, sizeof(UINT));
}

void Material::DefaultMaterial(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	shared_ptr<VS_MaterialMappedFormat> pMappedMaterial;
	UINT cbElementSize = (sizeof(VS_MaterialMappedFormat) + 255) & (~255);
	ComPtr<ID3D12Resource> temp;

	pMaterialBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pMaterialBuffer->Map(0, NULL, (void**)&pMappedMaterial);

	XMFLOAT4 ambient = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT4 diffuse = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT4 specular = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT4 emissive = XMFLOAT4(0,0,0,1);
	nType = 1;
	// format�� �´°��� ���Ͽ��� �о� ���ҽ��� ����

	memcpy(&pMappedMaterial->ambient, &ambient, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->diffuse, &diffuse, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->specular, &specular, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->emissive, &emissive, sizeof(XMFLOAT4));
	memcpy(&pMappedMaterial->nTypes, &nType, sizeof(UINT));
}