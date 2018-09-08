#include "CameraCtrl.h"
#include "d3dApp.h"
using namespace std::placeholders;

CameraCtrlRotate::CameraCtrlRotate(D3DApp& app, float radiusMin,float radiusMax)
	:mpD3dApp(&app)
	,mRadiusRange(radiusMin,radiusMax)
	,mTheta(0.f)
	,mPhi(0.4f* MathHelper::Pi)
	,mRadius(0.5f *(radiusMin +radiusMax))
	,mEyePosW(0,0,0)
{
	app.GetEventOnMouseDown().Add(std::bind(&CameraCtrlRotate::OnMouseDown,this,_1,_2,_3));
	app.GetEventOnMouseMove().Add(std::bind(&CameraCtrlRotate::OnMouseMove,this,_1,_2,_3));
	app.GetEventOnMouseUp().Add(std::bind(&CameraCtrlRotate::OnMouseUp,this,_1,_2,_3));
}

void CameraCtrlRotate::UpdateCamera(Camera& camera)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW.x = x;
	mEyePosW.y = y;
	mEyePosW.z = z;

	XMVECTOR pos = XMVectorSet(x,y,z,1);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0,1,0,0);

	camera.LookAt(pos,target,up);
	camera.UpdateViewMatrix();
}

void CameraCtrlRotate::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mpD3dApp->MainWnd());
}

void CameraCtrlRotate::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CameraCtrlRotate::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		float rate = (mRadiusRange.y - mRadiusRange.x)/5000;

		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = rate*static_cast<float>(x - mLastMousePos.x);
		float dy = rate*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, mRadiusRange.x, mRadiusRange.y);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}