#include "stdafx.h"
#include "Texture.h"
#include "GameObject.h"


Texture::Texture(int _nTexture, UINT _textureType, int _nSampler, int _nRootParameter) {
	
	// nTextures : �ؽ�ó�� ����
	// nTextureType : �ؽ�ó�� Ÿ��
	// nSamplers : ���÷��� ����. ���� ���÷��� ���� ����� ������� �ʴ´�
	// nRootParameters : ��Ʈ �Ķ������ ����
	// ���� �ڵ忡���� �˺��� �� �ϳ��� ����ϹǷ� Texture(1,RESOURCE_TEXTURE2D,0,1);�� �ɰ��̴�.
	textureType = _textureType;

	nTexture = _nTexture;
	if (nTexture > 0)
	{
		pTextureBuffers.assign(_nTexture, {});
		pTextureUploadBuffers.assign(_nTexture, {});
		name.assign(_nTexture, {});
		srvGpuDescriptorHandles.resize(_nTexture);
		for (int i = 0; i < _nTexture; i++) srvGpuDescriptorHandles[i].ptr = NULL;

		resourceTypes.resize(_nTexture);
		bufferFormats.resize(_nTexture);
		bufferElements.resize(_nTexture);
	}
	
	nRootParameter = _nRootParameter;
	
	if (nRootParameter > 0) rootParameterIndices.resize(_nRootParameter + 1);
	
	for (int i = 0; i < nRootParameter; i++) rootParameterIndices[i] = -1;

	nSampler = _nSampler;
	if (nSampler > 0) samplerGpuDescriptorHandles.resize(_nSampler);
}

Texture::~Texture() {
	
}

void Texture::SetRootParameterIndex(int _index, UINT _nRootParameterIndex)
{
	rootParameterIndices[_index] = _nRootParameterIndex;
}


void Texture::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> _pCommandList) {
	if (nRootParameter == nTexture)
	{
		for (int i = 0; i < nRootParameter; i++)
		{
			if (srvGpuDescriptorHandles[i].ptr && (rootParameterIndices[i] != -1)) {
				
				// �ش� ��°�� ��Ʈ �Ķ���� �ε���, ������ ���� �ε��� 
				_pCommandList->SetGraphicsRootDescriptorTable(rootParameterIndices[i], srvGpuDescriptorHandles[i]);
			}
		}
	}
	else
	{
		if (srvGpuDescriptorHandles[0].ptr) {
			_pCommandList->SetGraphicsRootDescriptorTable(rootParameterIndices[0], srvGpuDescriptorHandles[0]);
		}
	}
}

bool Texture::LoadFromFile(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _resourceType, int _index, int _startRootSignatureIndex) {

	// �ؽ�ó �ε�
	// �˺��� �ʸ� ���.
	name[_index] = _name;

	wstring tmpName = wstring(name[_index].begin(), name[_index].end());
	wstring filename = L"Texture/" + tmpName + L".dds";
	// �ؽ�ó ������ ���°�� false
	ifstream in(filename);
	if (!in) return false;
	
	resourceTypes[_index] = _resourceType;

	pTextureBuffers[_index] = CreateTextureResourceFromDDSFile(_pDevice.Get(), _pCommandList.Get(), filename.c_str(), pTextureUploadBuffers[_index].GetAddressOf(), D3D12_RESOURCE_STATE_GENERIC_READ);
	// �Ϲ� ������Ʈ�� ���⼭ SRV�� ����. 

	//if (_name.substr(0,4) != "2DUI" && _name != "water" && _name.substr(0, 6) != "Skybox" && _startRootSignatureIndex == 4)
	//{
	//	cout << _name.substr(0, 6) << "\n";
	//	auto pShader = Mesh::GetShader();
	//	pShader->CreateShaderResourceViews(_pDevice, shared_from_this(), _index, _startRootSignatureIndex);
	//}
	// �Ϲ� �ؽ�ó�� 4��, ������ 5���� ����Ѵ�.	

	//rootParameterIndices[_index] = _startRootSignatureIndex + _index;

	return true;

}

void Texture::ReleaseUploadBuffers() {
	for (int i = 0; i < nTexture; i++) {
		if(pTextureUploadBuffers[i]) pTextureUploadBuffers[i].Reset();
	}
}

void Texture::SetGpuDescriptorHandle(int _index, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle)
{
	// �ؽ�ó�� GPU ��ũ���� �ڵ��� �����Ѵ�.
	srvGpuDescriptorHandles[_index] = srvGpuDescriptorHandle;
}


D3D12_SHADER_RESOURCE_VIEW_DESC Texture::GetShaderResourceViewDesc(int _index)
{
	ComPtr<ID3D12Resource> pShaderResource = GetResource(_index);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(_index);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = 1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = bufferFormats[_index];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = bufferElements[_index];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}


/////////////////////////// TextureManager ////////////////////////


shared_ptr<Texture> TextureManager::GetTexture(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	if (!storage.contains(_name)) {	// ó�� �ҷ��� �ؽ�ó�� ���
		shared_ptr<Texture> newTexture = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
		
		bool result = newTexture->LoadFromFile(_name, _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
		
		if (!result) return nullptr;
		storage[_name] = newTexture;
	}
	return storage[_name];
}

void TextureManager::ReleaseUploadBuffers() {
	for (auto& pTexture : storage) {
		pTexture.second->ReleaseUploadBuffers();
	}
}