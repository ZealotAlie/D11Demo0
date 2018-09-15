#ifndef _BASIC32_H_
#define _BASIC32_H_

#include "../d3dUtil.h"
#include "../Effect.h"
#include "../d3dApp.h"
#include "UnknownInstance.h"

struct Basic32
{
	Basic32() : Pos(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 0.0f), Tex(0.0f, 0.0f) {}
	Basic32(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT2& uv)
		: Pos(p), Normal(n), Tex(uv) {}
	Basic32(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
		: Pos(px, py, pz), Normal(nx, ny, nz), Tex(u,v) {}
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;

	static ID3D11InputLayout* GetInputLayout();
};


class BasicEffect : public Effect,public UnknownInstance<BasicEffect>
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& filename);
	~BasicEffect();

	ID3DX11EffectTechnique* GetTechnique( RenderOptions option )
	{
		switch (option)
		{
		case Lighting:
			return Light3Tech;
		case Textures:
			return Light3TexTech;
		case TexturesAndFog:
		default:
			return Light3TexFogTech;
		}
	}

	void SetWorldViewProj(CXMMATRIX M)                  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M)                          { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M)              { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M)                   { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v)                  { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR v)                 { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f)                           { FogStart->SetFloat(f); }
	void SetFogRange(float f)                           { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights)   { DirLights->SetRawValue(lights, 0, 3*sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat)               { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetDiffuseMap(ID3D11ShaderResourceView* tex)   { DiffuseMap->SetResource(tex); }
	void SetCubeMap(ID3D11ShaderResourceView* tex)      { CubeMap->SetResource(tex); }

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;

	ID3DX11EffectTechnique* Light0TexTech;
	ID3DX11EffectTechnique* Light1TexTech;
	ID3DX11EffectTechnique* Light2TexTech;
	ID3DX11EffectTechnique* Light3TexTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipTech;

	ID3DX11EffectTechnique* Light1FogTech;
	ID3DX11EffectTechnique* Light2FogTech;
	ID3DX11EffectTechnique* Light3FogTech;

	ID3DX11EffectTechnique* Light0TexFogTech;
	ID3DX11EffectTechnique* Light1TexFogTech;
	ID3DX11EffectTechnique* Light2TexFogTech;
	ID3DX11EffectTechnique* Light3TexFogTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

	ID3DX11EffectTechnique* Light1ReflectTech;
	ID3DX11EffectTechnique* Light2ReflectTech;
	ID3DX11EffectTechnique* Light3ReflectTech;

	ID3DX11EffectTechnique* Light0TexReflectTech;
	ID3DX11EffectTechnique* Light1TexReflectTech;
	ID3DX11EffectTechnique* Light2TexReflectTech;
	ID3DX11EffectTechnique* Light3TexReflectTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipReflectTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipReflectTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipReflectTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipReflectTech;

	ID3DX11EffectTechnique* Light1FogReflectTech;
	ID3DX11EffectTechnique* Light2FogReflectTech;
	ID3DX11EffectTechnique* Light3FogReflectTech;

	ID3DX11EffectTechnique* Light0TexFogReflectTech;
	ID3DX11EffectTechnique* Light1TexFogReflectTech;
	ID3DX11EffectTechnique* Light2TexFogReflectTech;
	ID3DX11EffectTechnique* Light3TexFogReflectTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipFogReflectTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipFogReflectTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipFogReflectTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogReflectTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;
};

#endif