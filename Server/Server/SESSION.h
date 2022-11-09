#pragma once
class SESSION {
private:
	USHORT id;
	SOCKET socket;
	XMFLOAT4X4 transform;

public:		
	// ������ & �Ҹ���
	SESSION(USHORT _id, SOCKET& _socket);
	virtual ~SESSION();

	// get, set�Լ�
	const XMFLOAT4X4& GetTransform() const;
	void SetTransform(const XMFLOAT4X4& _transform);

};