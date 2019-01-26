#ifndef _SCENEOBJECT_H_
#define _SCENEOBJECT_H_

#include <wrl\client.h>
#include "d3dUtil.h"
#include "d3dx11effect.h"
#include "d3dApp.h"

template<typename T>
struct ConstRValue
{
public:
	ConstRValue(const T& d):value(d){}
	const T& value;
};

#define EFFECT_TRY_DEFINE(funcName)															\
public:																						\
	template <class C, typename P>															\
	static void funcName##(C c, const P& p)													\
	{																						\
		ConstRValue<P> warp(p);																\
		_##funcName##(c,warp);																\
	}																						\
private:																					\
	static void _##funcName##(...){}														\
																							\
	template <class C, typename P>															\
	static auto _##funcName##(C c, const P& p) -> decltype((void)(c->funcName(p.value)), void())	\
	{																						\
		c->funcName(p.value);																\
	}

class EffectSetter
{
public:
	
	EFFECT_TRY_DEFINE( SetWorld					);
	EFFECT_TRY_DEFINE( SetWorldView				);
	EFFECT_TRY_DEFINE( SetViewProj				);
	EFFECT_TRY_DEFINE( SetWorldInvTranspose		);
	EFFECT_TRY_DEFINE( SetWorldInvTransposeView );
	EFFECT_TRY_DEFINE( SetWorldViewProj			);
	EFFECT_TRY_DEFINE( SetWorldViewProjTex		);
	EFFECT_TRY_DEFINE( SetShadowTransform		);
	EFFECT_TRY_DEFINE( SetTexTransform			);
	EFFECT_TRY_DEFINE( SetMaterial				);
	EFFECT_TRY_DEFINE( SetDiffuseMap			);
	EFFECT_TRY_DEFINE( SetNormalMap				);
};

class SceneObject
{
public:
	SceneObject()
	{
		XMMATRIX I = XMMatrixIdentity();
		XMStoreFloat4x4(&mWorld, I);
		XMStoreFloat4x4(&mTexTransform, I);
	}

	XMFLOAT4X4& GetWorldTranform()		{ return mWorld; }
	XMFLOAT4X4& GetTexTransform()		{ return mTexTransform; }
	XMFLOAT4X4& GetShadowTransform()	{ return mShadowTransform; }
	Material& GetMaterial()				{ return mMaterial; }

	void SetTexture(ID3D11ShaderResourceView* texture)
	{
		mTextureSRV = texture;
	}
	void SetNormalTexture(ID3D11ShaderResourceView* texture)
	{
		mNormalTextureSRV = texture;
	}

	void SetBufferOffset(UINT indexNum,UINT indexFirst = 0,UINT vertexFirst = 0)
	{
		mIndexFirst		= indexFirst;
		mIndexCount		= indexNum;
		mVertexFirst	= vertexFirst;
	}

	template< class TEffect >
	void Draw(D3DApp* pApp, TEffect* pEffect, ID3DX11EffectTechnique* pTech, XMFLOAT4X4* pView = NULL, XMFLOAT4X4* pProj = NULL)
	{
		const Camera& camera = pApp->GetCamera();

		XMMATRIX view     = pView == NULL ? camera.View():XMLoadFloat4x4(pView);
		XMMATRIX proj     = pProj == NULL ? camera.Proj():XMLoadFloat4x4(pProj);
		XMMATRIX viewProj = view*proj;

		XMMATRIX world;
		XMMATRIX worldInvTranspose;
		XMMATRIX worldInvTransposeView;
		XMMATRIX worldView;
		XMMATRIX worldViewProj;

		world = XMLoadFloat4x4(&mWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView = world*view;
		worldViewProj = worldView*proj;
		worldInvTransposeView = worldInvTranspose*view;

		XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);
		XMMATRIX toTexSpace(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);
		XMMATRIX textTransform = XMLoadFloat4x4(&mTexTransform);

		EffectSetter::SetWorld					(pEffect, world);
		EffectSetter::SetWorldInvTranspose		(pEffect, worldInvTranspose);
		EffectSetter::SetWorldInvTransposeView	(pEffect, worldInvTransposeView);
		EffectSetter::SetWorldView				(pEffect, worldView);
		EffectSetter::SetViewProj				(pEffect, viewProj);
		EffectSetter::SetWorldViewProj			(pEffect, worldViewProj);
		EffectSetter::SetWorldViewProjTex		(pEffect, worldViewProj*toTexSpace);
		EffectSetter::SetShadowTransform		(pEffect, shadowTransform);
		EffectSetter::SetTexTransform			(pEffect, textTransform);
		EffectSetter::SetMaterial				(pEffect, mMaterial);
		EffectSetter::SetDiffuseMap				(pEffect, mTextureSRV.Get());
		EffectSetter::SetNormalMap				(pEffect, mNormalTextureSRV.Get());


		D3DX11_TECHNIQUE_DESC techDesc;
		pTech->GetDesc( &techDesc );
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			pTech->GetPassByIndex(p)->Apply(0, pApp->GetContext());
			pApp->GetContext()->DrawIndexed(mIndexCount, mIndexFirst, mVertexFirst);
		}
	}

private:
	XMFLOAT4X4	mWorld;
	XMFLOAT4X4	mTexTransform;
	XMFLOAT4X4	mShadowTransform;

	Material	mMaterial;

	UINT		mIndexFirst;
	UINT		mIndexCount;
	UINT		mVertexFirst;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	mTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	mNormalTextureSRV;
};

#endif