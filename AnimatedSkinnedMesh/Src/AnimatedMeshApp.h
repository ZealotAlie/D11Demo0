#pragma once

#include "d3dApp.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Camera.h"
#include "Effect.h"
#include "CommonRenderObjects.h"
#include "UnknownInstance\Basic32.h"
#include "TextureMgr.h"
#include "LoadM3d.h"
#include "SkinnedModel.h"

struct SBoundingSphere
{
	SBoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

class AnimatedMeshApp : public D3DApp
{
public:
	AnimatedMeshApp(HINSTANCE hInstance);
	~AnimatedMeshApp();

	bool Init()override;
	void DrawScene()override;
	void UpdateScene(float dt)override;
	void OnResize()override;

private:
	CommonEnvironment		mEnvironment;
	CameraCtrlRotate		mRotateCameraController;
	SBoundingSphere			mSceneBounds;

	XMFLOAT4X4				mLightView;
	XMFLOAT4X4				mLightProj;

	SkyObject				mSky;

	TextureMgr				mTextureMgr;
	M3DLoader				mLoader;
	SkinnedModel*			mCharacterModel;
	SkinnedModelInstance	mCharacterInstance;
};