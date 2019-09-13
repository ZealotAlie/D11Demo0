#pragma once

#include "Effect.h"
#include "UnknownInstance\UnknownInstance.h"

struct PosNormalTexTanSkinned
{
	XMFLOAT3	Pos;
	XMFLOAT3	Normal;
	XMFLOAT2	Tex;
	XMFLOAT4	TangentU;
	XMFLOAT3	Weights;
	BYTE		BoneIndices[4];

	static ID3D11InputLayout* GetInputLayout();
};

class SkinnedEffect : public Effect, public UnknownInstance<SkinnedEffect>
{
public:
	SkinnedEffect(ID3D11Device* device, const std::wstring& filename);
	~SkinnedEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProjTex(CXMMATRIX M) { WorldViewProjTex->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetBoneTransforms(const XMFLOAT4X4* M, int cnt) { BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetNormalMap(ID3D11ShaderResourceView* tex) { NormalMap->SetResource(tex); }

	ID3DX11EffectTechnique* Light1SkinnedTech;
	ID3DX11EffectTechnique* Light2SkinnedTech;
	ID3DX11EffectTechnique* Light3SkinnedTech;

	ID3DX11EffectTechnique* Light0TexSkinnedTech;
	ID3DX11EffectTechnique* Light1TexSkinnedTech;
	ID3DX11EffectTechnique* Light2TexSkinnedTech;
	ID3DX11EffectTechnique* Light3TexSkinnedTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipSkinnedTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipSkinnedTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipSkinnedTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipSkinnedTech;

	ID3DX11EffectTechnique* Light1FogSkinnedTech;
	ID3DX11EffectTechnique* Light2FogSkinnedTech;
	ID3DX11EffectTechnique* Light3FogSkinnedTech;

	ID3DX11EffectTechnique* Light0TexFogSkinnedTech;
	ID3DX11EffectTechnique* Light1TexFogSkinnedTech;
	ID3DX11EffectTechnique* Light2TexFogSkinnedTech;
	ID3DX11EffectTechnique* Light3TexFogSkinnedTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipFogSkinnedTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipFogSkinnedTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipFogSkinnedTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogSkinnedTech;

	ID3DX11EffectMatrixVariable*	WorldViewProj;
	ID3DX11EffectMatrixVariable*	WorldViewProjTex;
	ID3DX11EffectMatrixVariable*	World;
	ID3DX11EffectMatrixVariable*	WorldInvTranspose;
	ID3DX11EffectMatrixVariable*	BoneTransforms;
	ID3DX11EffectMatrixVariable*	TexTransform;
	ID3DX11EffectVectorVariable*	EyePosW;
	ID3DX11EffectVectorVariable*	FogColor;
	ID3DX11EffectScalarVariable*	FogStart;
	ID3DX11EffectScalarVariable*	FogRange;
	ID3DX11EffectVariable*			DirLights;
	ID3DX11EffectVariable*			Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
};