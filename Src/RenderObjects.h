#ifndef _RENDEROBJECT_H_
#define _RENDEROBJECT_H_

#include "Vertex.h"
#include "Waves.h"
#include "d3dUtil.h"
#include "d3dx11effect.h"

class D3DApp;
class RenderObject
{
public:
	RenderObject()
		:mTextureSRV(nullptr)
		,mIndexBuffer(nullptr)
		,mVertexBuffer(nullptr)
	{
		XMMATRIX I = XMMatrixIdentity();
		XMStoreFloat4x4(&mWorld, I);
		XMStoreFloat4x4(&mTexTransform, I);
	}

	virtual ~RenderObject();

	ID3D11ShaderResourceView*	GetTextureSRV()const{ return mTextureSRV; }
	const Material&				GetMaterial()const{ return mMaterial; }
	const XMFLOAT4X4&			GetTexTransform()const { return mTexTransform; }
	const XMFLOAT4X4&			GetWorld()const{return mWorld;}

	virtual void				Update(float dt){}
	virtual void				Build(const D3DApp* pApp) = 0;
	virtual void				Draw(const D3DApp* pApp);
	virtual void				DrawTransparency(const D3DApp* pApp){};

	template < typename T >
	static void CreateBuffer( ID3D11Buffer*& pBuffer, ID3D11Device* pDevice, UINT size, T* pData, UINT bindFlags, D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE );

	template < typename TEffect >
	void						SetupEffect( TEffect* pEffect, const D3DApp* pApp ) const
	{
		XMMATRIX world = XMLoadFloat4x4(&mWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*pApp->GetCamera().ViewProj();

		pEffect->SetWorld(world);
		pEffect->SetWorldInvTranspose(worldInvTranspose);
		pEffect->SetWorldViewProj(worldViewProj);
		pEffect->SetMaterial( mMaterial );
		pEffect->SetDiffuseMap( mTextureSRV );
		pEffect->SetTexTransform( XMLoadFloat4x4( &mTexTransform ) );
		pEffect->SetEyePosW(pApp->GetCamera().GetPosition());
	}

protected:
	void						DrawWithTech(ID3D11DeviceContext* pContext,ID3DX11EffectTechnique* pTech)const;

protected:
	XMFLOAT4X4					mWorld;
	Material					mMaterial;
	XMFLOAT4X4					mTexTransform;

	ID3D11ShaderResourceView*	mTextureSRV;
	ID3D11Buffer*				mVertexBuffer;
	ID3D11Buffer*				mIndexBuffer;
	UINT						mIndexNum;
};

class MountObject:public RenderObject
{
public:
	virtual void	Build(const D3DApp* pApp)override;
};

class WaterObject:public RenderObject
{
public:
	WaterObject():mTBase(0),mTTotal(0),mNeedMap(false){}

	virtual void	Build(const D3DApp* pApp)override;
	virtual void	Update(float dt)override;
	virtual void	Draw(const D3DApp* pApp) override{}
	virtual void	DrawTransparency(const D3DApp* pApp) override;

private:
	Waves		mWaves;
	XMFLOAT2	mWaterTexOffset;

	float		mTBase;
	float		mTTotal;
	bool		mNeedMap;
};

class TreeObject:public RenderObject
{
public:
	virtual void	Build(const D3DApp* pApp)override;
	virtual void	Draw(const D3DApp* pApp)override;
};

class WallObject:public RenderObject
{
public:
	WallObject()
		:mFloorDiffuseMapSRV(nullptr)
		,mWallDiffuseMapSRV(nullptr)
		,mMirrorDiffuseMapSRV(nullptr)
	{}
	~WallObject();

	ID3D11Buffer*const* GetVertexBuffer()const{return &mVertexBuffer;}

	virtual void	Build(const D3DApp* pApp)override;
	virtual void	Draw(const D3DApp* pApp)override;
	virtual void	DrawTransparency(const D3DApp* pApp)override;
private:
	ID3D11ShaderResourceView*	mFloorDiffuseMapSRV;
	ID3D11ShaderResourceView*	mWallDiffuseMapSRV;
	ID3D11ShaderResourceView*	mMirrorDiffuseMapSRV;

	Material					mMirrorMaterial;
};

class SkullObject:public RenderObject
{
public:
	SkullObject(WallObject* wall)
		:mWall(wall)
	{}

	virtual void	Build(const D3DApp* pApp)override;
	virtual void	Draw(const D3DApp* pApp)override;
	virtual void	DrawTransparency(const D3DApp* pApp)override;
	virtual void	Update(float dt)override;

private:
	XMFLOAT3		mSkullTranslation;
	WallObject*		mWall;
	Material		mShadowMat;
};

class FullScreenQuadObject:public RenderObject
{
public:
	void Build(const D3DApp* pApp)override;
	void DrawQuad(const D3DApp* pApp,ID3D11ShaderResourceView* tex)const;

private:
	virtual void Draw(const D3DApp* pApp)override{}
	virtual void DrawTransparency(const D3DApp* pApp)override{}
};

class CsWaterObject:public RenderObject
{
	enum
	{
		VERTEX_NUM = 256,
	};

public:
	CsWaterObject();
	~CsWaterObject();
	void Build(const D3DApp* pApp)override;

	virtual void Draw(const D3DApp* pApp)override{}
	virtual void DrawTransparency(const D3DApp* pApp)override;
	virtual void Update(float dt)override;

private:
	void DrawUpdateWave(ID3D11DeviceContext* pD3dContext) const;
	void DrawDisturb(ID3D11DeviceContext* pContext) const;

private:
	ID3D11UnorderedAccessView*	mWaterOutputUAV;
	ID3D11UnorderedAccessView*	mWaveHeightUAV;

	ID3D11Buffer*				mWaterOutputBuffer;

	XMFLOAT2					mWaterTexOffset;

	float						mDisturbCounter;
	float						mWaveCounter;
};
#endif