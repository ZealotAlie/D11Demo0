#ifndef _NDSSAOAPP_H_
#define _NDSSAOAPP_H_

#include "d3dApp.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Camera.h"
#include "Effect.h"
#include "CommonRenderObjects.h"
#include "Ssao.h"
#include "SceneObject.h"
#include "ShadowMap.h"
#include "VertexBufferState.h"
#include "UnknownInstance\Basic32.h"
#include "NormalDMapEffect.h"

struct SBoundingSphere
{
	SBoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

enum EDrawOptions
{
	RenderOptionsBasic = 0,
	RenderOptionsNormalMap = 1,
	RenderOptionsDisplacementMap = 2
};

class NDSsaoApp : public D3DApp 
{
public:
	NDSsaoApp(HINSTANCE hInstance);
	~NDSsaoApp();

	bool Init()override;
	void DrawScene()override; 
	void UpdateScene(float dt)override;
	void OnResize()override;

private:
	void BuildShadowTransform();
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();
	void BuildScreenQuadGeometryBuffers();

	void DrawSceneToShadowMap();
	void DrawSceneToSsaoNormalDepthMap();

	void DrawScreenQuad(ID3D11ShaderResourceView* srv);

	void DrawWithOption(EDrawOptions option,SceneObject& obj, bool drawReflect);
private:
	CommonEnvironment		mEnvironment;
	CameraCtrlRotate		mRotateCameraController;
	SBoundingSphere			mSceneBounds;
	ShadowMap				mSmap;

	float					mLightRotationAngle;
	XMFLOAT3				mOriginalLightDir[3];


	XMFLOAT4X4				mLightView;
	XMFLOAT4X4				mLightProj;
	XMFLOAT4X4				mShadowTransform;

	SkyObject				mSky;
	Ssao					mSsao;

	SceneObject				mBox;
	SceneObject				mGrid;
	SceneObject				mSphere[10];
	SceneObject				mCylinders[10];

	SceneObject				mSkull;

	std::vector<SceneObject*>				mSceneObjects;

	VertexBufferState<PosNormalTexTan>		mShapesVertexState;
	VertexBufferState<Basic32>				mSkullVertexState;
	VertexBufferState<Basic32>				mScreenVertexState;

	EDrawOptions							mDrawOptions;
};

#endif