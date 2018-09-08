#ifndef _CAMERACTRLBASE_H_
#define _CAMERACTRLBASE_H_

#include "d3dUtil.h"

class D3DApp;
class Camera;
class CameraCtrlBase
{
public:
	CameraCtrlBase()
		: mFovY(0.25f*MathHelper::Pi)
		, mZFar(1000.0f)
		, mZNear(1.f)
	{}

	virtual~CameraCtrlBase(){}

	float GetFovY()		const{ return mFovY;	}
	float GetZFar()		const{ return mZFar;	}
	float GetZNear()	const{ return mZNear;	}

	virtual void UpdateCamera(Camera& camera) = 0;

protected:
	float mFovY;
	float mZFar;
	float mZNear;
};

class CameraCtrlRotate:public CameraCtrlBase
{
public:
	CameraCtrlRotate( D3DApp& app, float radiusMin,float radiusMax );
	void UpdateCamera(Camera& camera)override;

private:
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

	XMFLOAT3	mEyePosW;
	XMFLOAT2	mRadiusRange;

	float		mTheta;
	float		mPhi;
	float		mRadius;

	POINT		mLastMousePos;
	D3DApp*		mpD3dApp;
};

#endif