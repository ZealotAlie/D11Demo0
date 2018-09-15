#ifndef _INSTANCEDSKULL_H_
#define _INSTANCEDSKULL_H_

#include "d3dUtil.h"
#include "InstancedData.h"
#include "CommonRenderObjects.h"

enum FrustumCullingType
{
	CullingNone,
	LocalFrustumCulling,
	CameraFrustumCulling,
};

class InstancedSkull :public BasicSkullObject
{
public:
	InstancedSkull()
		:mInstanceVB(nullptr)
		,mCullingType(CullingNone)
		,mVisibleObjectCount(0)
	{
		mMaterial.Ambient  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		mMaterial.Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		mMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	}

	~InstancedSkull()
	{
		ReleaseCOM(mInstanceVB);
	}

	virtual void	Update(float dt)override;

	virtual void	Build(const D3DApp* pApp)override;
	virtual void	Draw(const D3DApp* pApp)override;

	void SetFrustumCullingType( FrustumCullingType type )
	{
		mCullingType = type;
	}
	UINT GetVisibleObjectCount()const
	{
		return mVisibleObjectCount;
	}

private:
	void			BuildInstanceBuffer( ID3D11Device* pDevice );

private:
	ID3D11Buffer*				mInstanceVB;
	std::vector<InstancedData>	mInstancedData;
	FrustumCullingType			mCullingType;
	UINT						mVisibleObjectCount;
};

#endif