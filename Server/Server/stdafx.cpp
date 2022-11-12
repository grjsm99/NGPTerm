#include "stdafx.h"

unordered_map<USHORT, SESSION> clients;
int cid, mid;

SESSION::SESSION(USHORT _id, SOCKET& _socket) {
	id = _id;
	socket = _socket;
	transform = Matrix4x4::Identity();
}

SESSION::~SESSION() {

}

const XMFLOAT4X4& SESSION::GetTransform() const {
	return transform;
}

void SESSION::SetTransform(const XMFLOAT4X4& _transform) {
	transform = _transform;
}
