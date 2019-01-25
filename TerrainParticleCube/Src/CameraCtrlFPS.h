#ifndef _CAMERACTRLFPS_H_
#define _CAMERACTRLFPS_H_

#include "CameraCtrl.h"

class Terrain;
class CameraCtrlFPS:public CameraCtrlBase
{
public:
	CameraCtrlFPS(const Terrain* terrain);
	void UpdateCamera(Camera& camera)override;

private:
	void OnMouseDown	(WPARAM btnState, int x, int y);
	void OnMouseUp		(WPARAM btnState, int x, int y);
	void OnMouseMove	(WPARAM btnState, int x, int y);

	XMFLOAT3			mPosition;
	XMFLOAT2			mRadiusRange;

	float				mTheta;
	float				mPhi;

	POINT				mLastMousePos;
	const Terrain*		mTerrain;
};

#endif