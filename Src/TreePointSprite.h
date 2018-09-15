#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "d3dUtil.h"

struct TreePointSprite
{
	XMFLOAT3 Pos;
	XMFLOAT2 Size;

	static ID3D11InputLayout* GetInputLayout();
};

#endif