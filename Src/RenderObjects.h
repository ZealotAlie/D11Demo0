#ifndef _RENDEROBJECTS_H_
#define _RENDEROBJECTS_H_

#include "Waves.h"
#include "CommonRenderObjects.h"

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

class SkullObject:public BasicSkullObject
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