#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "d3dUtil.h"
#include "d3dx11effect.h"
#include "LightHelper.h"
#include <DirectXColors.h>

class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

	Effect* GetResource(){ return this; }

private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};

struct CommonEnvironment
{
	DirectionalLight			dirLights[3];
	XMVECTORF32					fogColor;
	float						fogStart;
	float						fogRange;

	CommonEnvironment()
		: fogColor( Colors::White )
		, fogStart( 0 )
		, fogRange( 0 )
	{
		dirLights[0].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		dirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		dirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		dirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

		dirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		dirLights[1].Diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
		dirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
		dirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

		dirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		dirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		dirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		dirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
	}

	template<typename T>
	void Apply( T* pEffect )
	{
		pEffect->SetDirLights(dirLights);
		pEffect->SetFogColor(fogColor);
		pEffect->SetFogStart(fogStart);
		pEffect->SetFogRange(fogRange);
	}
};

#endif