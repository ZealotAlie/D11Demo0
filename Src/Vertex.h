#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "d3dUtil.h"

struct Vertex
{
	Vertex() : Pos(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 0.0f), Tex(0.0f, 0.0f) {}
	Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT2& uv)
		: Pos(p), Normal(n), Tex(uv) {}
	Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
		: Pos(px, py, pz), Normal(nx, ny, nz), Tex(u,v) {}
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

struct TreePointSprite
{
	XMFLOAT3 Pos;
	XMFLOAT2 Size;
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* Vertex;
	static ID3D11InputLayout* TreePointSprite;
};

#endif