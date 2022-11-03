#pragma once
class Texture : public enable_shared_from_this<Texture>
{
public:
	Texture(int _nTexture, UINT textureType, int nSampler, int nRootParameter);
	~Texture();
private:
	vector<string> name;
	UINT textureType;
	int nTexture;
	
	
	vector<ComPtr<ID3D12Resource>> pTextureBuffers;	 // �ؽ�ó �ʿ� ���� ���ҽ� ����
	vector<ComPtr<ID3D12Resource>> pTextureUploadBuffers;
	
	vector<UINT> resourceTypes;
	vector<DXGI_FORMAT> bufferFormats;
	vector<int> bufferElements;
	
	int	nRootParameter = 0;
	vector<int> rootParameterIndices;
	
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> srvGpuDescriptorHandles;
	
	int nSampler;
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> samplerGpuDescriptorHandles;
	
	
public:
	void SetRootParameterIndex(int _index, UINT _nRootParameterIndex);
	void UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> _pCommandList);
	bool LoadFromFile(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _resourceType, int _index, int _startRootSignatureIndex);
	
	void ReleaseUploadBuffers();
	
	void SetGpuDescriptorHandle(int _index, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle);
	
	// �ش� �ε����� �ؽ�ó ���ҽ��� ��ȯ
	ComPtr<ID3D12Resource> GetResource(int _index) { return pTextureBuffers[_index]; };
	// �ش� �ε����� �ؽ�ó�� ������ �ڵ��� ��ȯ
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int _index) { return(srvGpuDescriptorHandles[_index]); }
	// 
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int _index);
	UINT GetTextureType(int _index) { return(resourceTypes[_index]); }
	int GetnTexture() { return(nTexture); }
	int GetnRootParameter() { return(nRootParameter); };
	

};

class TextureManager {
	unordered_map<string, shared_ptr<Texture>> storage;
public:
	shared_ptr<Texture> GetTexture(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void ReleaseUploadBuffers();
};