#ifndef _TPCAPP_H_
#define _TPCAPP_H_

#include "d3dApp.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Camera.h"
#include "Effect.h"
#include "CommonRenderObjects.h"
#include "Terrain.h"
#include "CameraCtrlFPS.h"
#include "ParticleSystem.h"

class TPCApp : public D3DApp 
{
public:
	TPCApp(HINSTANCE hInstance);
	~TPCApp();

	bool Init();
	void DrawScene(); 
	void UpdateScene(float dt)override;

private:
	CommonEnvironment		mEnvironment;
	CameraCtrlFPS			mCameraCtrlFPS;
	CameraCtrlRotate		mRotateCameraController;
	SkyObject				mSky;
	Terrain					mTerrain;

	ID3D11ShaderResourceView*	mFlareTexSRV;
	ID3D11ShaderResourceView*	mRainTexSRV;
	ID3D11ShaderResourceView*	mRandomTexSRV;

	ParticleSystem				mFire;
	ParticleSystem				mRain;
};


#endif