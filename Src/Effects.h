#ifndef EFFECTS_H
#define EFFECTS_H

#include "d3dUtil.h"
#include "d3dx11effect.h"
#include "d3dApp.h"

class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};

class BasicEffect : public Effect
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
};

class TreeSpriteEffect : public Effect
{
public:
	TreeSpriteEffect(ID3D11Device* device, const std::wstring& filename);
	~TreeSpriteEffect();

	ID3DX11EffectTechnique* GetTechnique( RenderOptions option )
	{
		switch (option)
		{
		case Lighting:
		case Textures:
			return Light3TexAlphaClipTech;
		case TexturesAndFog:
		default:
			return Light3TexAlphaClipFogTech;
		}
	}

	void SetViewProj(CXMMATRIX M)                              { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v)                         { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR v)                        { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f)                                  { FogStart->SetFloat(f); }
	void SetFogRange(float f)                                  { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights)          { DirLights->SetRawValue(lights, 0, 3*sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat)                      { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetTreeTextureMapArray(ID3D11ShaderResourceView* tex) { TreeTextureMapArray->SetResource(tex); }

	ID3DX11EffectTechnique* Light3Tech;
	ID3DX11EffectTechnique* Light3TexAlphaClipTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* TreeTextureMapArray;
};
class BlurEffect:public Effect
{
public:
	BlurEffect(ID3D11Device* device, const std::wstring& filename);
	~BlurEffect();

	void SetWeights(const float (&weights)[11])           { Weights->SetFloatArray(weights, 0, ARRAYSIZE(weights)); }
	void SetInputMap(ID3D11ShaderResourceView* tex)   { InputMap->SetResource(tex); }
	void SetOutputMap(ID3D11UnorderedAccessView* tex) { OutputMap->SetUnorderedAccessView(tex); }

	ID3DX11EffectTechnique* HorzBlurTech;
	ID3DX11EffectTechnique* VertBlurTech;

	ID3DX11EffectScalarVariable* Weights;
	ID3DX11EffectShaderResourceVariable* InputMap;
	ID3DX11EffectUnorderedAccessViewVariable* OutputMap;
};

class WaterEffect: public Effect
{
public:
	WaterEffect(ID3D11Device* device, const std::wstring& filename);
	~WaterEffect();

	ID3DX11EffectTechnique* GetTechnique( RenderOptions option )
	{
		switch (option)
		{
		case Lighting:
			return WaterTech;
		case Textures:
			return WaterTexTech;
		case TexturesAndFog:
		default:
			return WaterTexFog;
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

	void SetWaveOutput(ID3D11UnorderedAccessView* opt)	{ WaveOutput->SetUnorderedAccessView(opt); }
	void SetWavePreY(ID3D11UnorderedAccessView* opt)	{ WavePreY->SetUnorderedAccessView(opt); }
	void SetWaveSize(const XMINT2& size) 				{ WaveSize->SetRawValue(&size, 0, sizeof(size));}
	void SetDisturbHeight(float height)					{ DisturbHeight->SetFloat(height); }
	void SetDisturbIndex(const XMINT2& disturbID)		{ DisturbIndex->SetRawValue(&disturbID, 0, sizeof(disturbID));}

	ID3DX11EffectTechnique* WaterTech;
	ID3DX11EffectTechnique* WaterTexTech;
	ID3DX11EffectTechnique* WaterTexFog;
	ID3DX11EffectTechnique* WaveMotionTech;
	ID3DX11EffectTechnique* WaveDisturbTech;

	ID3DX11EffectScalarVariable* DisturbHeight;
	ID3DX11EffectVectorVariable* DisturbIndex;
	ID3DX11EffectVectorVariable* WaveSize;
	ID3DX11EffectUnorderedAccessViewVariable* WaveOutput;
	ID3DX11EffectUnorderedAccessViewVariable* WavePreY;

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
};

class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();
	template<typename T>
	static void SetupLightFog(T* pEffect, const DirectionalLight* pLights, const FXMVECTOR fogColor, float fogStart, float fogRange)
	{
		pEffect->SetDirLights(pLights);
		pEffect->SetFogColor(fogColor);
		pEffect->SetFogStart(fogStart);
		pEffect->SetFogRange(fogRange);
	}

	static BasicEffect*		BasicFX;
	static TreeSpriteEffect*TreeSprite;
	static BlurEffect*		BlurFX;
	static WaterEffect*		Water;
};
#endif