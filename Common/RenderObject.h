#ifndef _RENDEROBJECT_H_
#define _RENDEROBJECT_H_

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
	static void					CreateBuffer(ID3D11Buffer*& pBuffer, ID3D11Device* pDevice, UINT size, T* pData, UINT bindFlags, D3D11_USAGE usage  = D3D11_USAGE_IMMUTABLE );
	template < typename TEffect >
	void						SetupEffect( TEffect* pEffect, const D3DApp* pApp ) const;

protected:
	void						DrawWithTech(ID3D11DeviceContext* pContext,ID3DX11EffectTechnique* pTech)const;

protected:
	XMFLOAT4X4					mWorld;
	XMFLOAT4X4					mTexTransform;

	Material					mMaterial;

	ID3D11ShaderResourceView*	mTextureSRV;
	ID3D11Buffer*				mVertexBuffer;
	ID3D11Buffer*				mIndexBuffer;
	UINT						mIndexNum;
};

template < typename T >
void RenderObject::CreateBuffer(ID3D11Buffer*& pBuffer, ID3D11Device* pDevice, UINT size, T* pData, UINT bindFlags, D3D11_USAGE usage /*= D3D11_USAGE_IMMUTABLE */)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = usage;
	vbd.BindFlags = bindFlags;
	vbd.ByteWidth = sizeof(T)*size;
	vbd.StructureByteStride = sizeof(T);
	vbd.MiscFlags = 0;
	UINT cpuAccessFlags = 0;
	if( usage == D3D11_USAGE_DYNAMIC )
	{
		cpuAccessFlags |= D3D11_CPU_ACCESS_WRITE;
	}
	if( usage == D3D11_USAGE_STAGING )
	{
		cpuAccessFlags |= D3D11_CPU_ACCESS_READ;
	}
	vbd.CPUAccessFlags = cpuAccessFlags;

	if(pData != nullptr)
	{
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pData;
		HR( pDevice->CreateBuffer( &vbd, &initData, &pBuffer ) );
	}
	else
	{
		HR( pDevice->CreateBuffer( &vbd, nullptr, &pBuffer ) );
	}
}

template < typename TEffect >
void RenderObject::SetupEffect(TEffect* pEffect, const D3DApp* pApp) const
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
	pEffect->SetEyePosW( pApp->GetCamera().GetPosition() );
}

#endif