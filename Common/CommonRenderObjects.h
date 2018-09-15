#ifndef _COMMONRENDEROBJECTS_H_
#define _COMMONRENDEROBJECTS_H_

#include "RenderObject.h"
#include <DirectXCollision.h>

class BasicSkullObject:public RenderObject
{
public:
	virtual void Build(const D3DApp* pApp)override;

protected:
	BoundingBox		mSkullBox;
};

class SkyObject:public RenderObject
{
	virtual void	Build(const D3DApp* pApp)override{ Assert(false,""); }
public:
	~SkyObject();
	void			Build(const D3DApp* pApp,const std::vector<std::wstring> cubemapFilenames, float skySphereRadius);
	virtual void	Draw(const D3DApp* pApp)override;

protected:
	std::vector<ID3D11ShaderResourceView*>	mSkyTextures;
};

class StoneBuildingBase:public RenderObject
{
public:
	StoneBuildingBase();
	~StoneBuildingBase();

protected:
	Material		mGridMat;
	Material		mBoxMat;
	Material		mCylinderMat;
	Material		mSphereMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4		mSphereWorld[10];
	XMFLOAT4X4		mCylWorld[10];
	XMFLOAT4X4		mBoxWorld;
	XMFLOAT4X4		mGridWorld;

	int				mBoxVertexOffset;
	int				mGridVertexOffset;
	int				mSphereVertexOffset;
	int				mCylinderVertexOffset;

	UINT			mBoxIndexOffset;
	UINT			mGridIndexOffset;
	UINT			mSphereIndexOffset;
	UINT			mCylinderIndexOffset;

	UINT			mBoxIndexCount;
	UINT			mGridIndexCount;
	UINT			mSphereIndexCount;
	UINT			mCylinderIndexCount;

	ID3D11ShaderResourceView* mFloorTexSRV;
	ID3D11ShaderResourceView* mStoneTexSRV;
	ID3D11ShaderResourceView* mBrickTexSRV;
};
#endif