#ifndef _Pos_H_
#define _Pos_H_

#include "../d3dUtil.h"
#include "../Effect.h"
#include "../d3dApp.h"
#include "UnknownInstance.h"

struct Pos
{
	static ID3D11InputLayout* GetInputLayout();
};


class SkyEffect :public Effect,public UnknownInstance<SkyEffect>
{
public:
	SkyEffect(ID3D11Device* device, const std::wstring& filename);

	void SetWorldViewProj(CXMMATRIX M)                  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetCubeMap(ID3D11ShaderResourceView* cubemap)  { CubeMap->SetResource(cubemap); }

	ID3DX11EffectTechnique* SkyTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;

	ID3DX11EffectShaderResourceVariable* CubeMap;
};

#endif