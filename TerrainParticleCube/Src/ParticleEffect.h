#ifndef _PARTICLEEFFECT_H_
#define _PARTICLEEFFECT_H_

#include "Effect.h"
#include "UnknownInstance/UnknownInstance.h"

struct ParticleData
{
	XMFLOAT3 InitialPos;
	XMFLOAT3 InitialVel;
	XMFLOAT2 Size;
	float Age;
	unsigned int Type;

	static ID3D11InputLayout* GetInputLayout();
};

class ParticleEffect : public Effect
{
public:
	ParticleEffect(ID3D11Device* device, const std::wstring& filename);
	~ParticleEffect();

	void SetViewProj(CXMMATRIX M)                       { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetGameTime(float f)                           { GameTime->SetFloat(f); }
	void SetTimeStep(float f)                           { TimeStep->SetFloat(f); }

	void SetEyePosW(const XMFLOAT3& v)                  { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetEmitPosW(const XMFLOAT3& v)                 { EmitPosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetEmitDirW(const XMFLOAT3& v)                 { EmitDirW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }

	void SetTexArray(ID3D11ShaderResourceView* tex)     { TexArray->SetResource(tex); }
	void SetRandomTex(ID3D11ShaderResourceView* tex)    { RandomTex->SetResource(tex); }

	ID3DX11EffectTechnique* StreamOutTech;
	ID3DX11EffectTechnique* DrawTech;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectScalarVariable* GameTime;
	ID3DX11EffectScalarVariable* TimeStep;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* EmitPosW;
	ID3DX11EffectVectorVariable* EmitDirW;
	ID3DX11EffectShaderResourceVariable* TexArray;
	ID3DX11EffectShaderResourceVariable* RandomTex;
};

class RainEffect:public ParticleEffect, public UnknownInstance<RainEffect>
{
public:
	RainEffect(ID3D11Device* device, const std::wstring& filename):ParticleEffect(device,filename){}
};

class FireEffect:public ParticleEffect, public UnknownInstance<FireEffect>
{
public:
	FireEffect(ID3D11Device* device, const std::wstring& filename):ParticleEffect(device,filename){}
};


#endif