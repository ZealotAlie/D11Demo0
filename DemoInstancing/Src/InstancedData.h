#ifndef _INSTANCEDDATA_H_
#define _INSTANCEDDATA_H_

#include "d3dUtil.h"

struct InstancedData
{
	XMFLOAT4X4	World;
	XMFLOAT4	Color;

	static ID3D11InputLayout* GetInputLayout();
};


#endif