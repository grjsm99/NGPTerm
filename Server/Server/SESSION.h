#pragma once
class SESSION {
private:
	USHORT id;
	SOCKET socket;
	XMFLOAT4X4 transform;

public:		
	// 생성자 & 소멸자
	SESSION(USHORT _id, SOCKET& _socket);
	virtual ~SESSION();

	// get, set함수
	const XMFLOAT4X4& GetTransform() const;
	void SetTransform(const XMFLOAT4X4& _transform);

};