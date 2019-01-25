#include "CameraCtrlFPS.h"
#include "Camera.h"
#include "d3dApp.h"
#include "Terrain.h"

using namespace std::placeholders;

CameraCtrlFPS::CameraCtrlFPS(const Terrain* terrain)
	: mTheta(0.f)
	, mPhi(0.4f* MathHelper::Pi)
	, mPosition(0,0,0)
	, mTerrain(terrain)
{
	auto pApp = D3DApp::GetInstance();
	pApp->GetEventOnMouseDown().Add(std::bind(&CameraCtrlFPS::OnMouseDown,this,_1,_2,_3));
	pApp->GetEventOnMouseMove().Add(std::bind(&CameraCtrlFPS::OnMouseMove,this,_1,_2,_3));
	pApp->GetEventOnMouseUp().Add(std::bind(&CameraCtrlFPS::OnMouseUp,this,_1,_2,_3));
}


void CameraCtrlFPS::UpdateCamera(Camera& camera)
{
	const float speed = 100.0f;
	float dt = D3DApp::GetInstance()->DeltaTime();

	float dtWalk = 0;
	if( GetAsyncKeyState('W') & 0x8000 )
		dtWalk += speed*dt;
	if( GetAsyncKeyState('S') & 0x8000 )
		dtWalk -=speed*dt;

	float dtStrafe = 0;
	if( GetAsyncKeyState('A') & 0x8000 )
		dtStrafe -= speed*dt;

	if( GetAsyncKeyState('D') & 0x8000 )
		dtStrafe += speed*dt;

	mPosition.x += camera.GetLook().x*dtWalk + camera.GetRight().x*dtStrafe;
	mPosition.z += camera.GetLook().z*dtWalk + camera.GetRight().z*dtStrafe;
	if(mTerrain!= nullptr)
	{
		mPosition.y = mTerrain->GetHeight(mPosition.x,mPosition.z) + 10;
	}

	XMFLOAT3 target = mPosition;
	target.x -= sinf(mPhi) * cosf(mTheta);
	target.z -= sinf(mPhi) * sinf(mTheta);
	target.y -= cosf(mPhi);

	camera.LookAt(mPosition,target,XMFLOAT3(0,1,0));
	camera.UpdateViewMatrix();
}

void CameraCtrlFPS::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CameraCtrlFPS::OnMouseUp(WPARAM btnState, int x, int y)
{

}

void CameraCtrlFPS::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.075f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}