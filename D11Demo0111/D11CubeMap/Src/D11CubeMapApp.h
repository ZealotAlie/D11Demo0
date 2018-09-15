#ifndef _INSTANCINGDEMOAPP_H_
#define _INSTANCINGDEMOAPP_H_

#include "d3dApp.h"
#include "Effect.h"
#include "RenderObject.h"
#include "CommonRenderObjects.h"
#include "StoneBuildingObject.h"

class RenderObject;
class D11CubeMapApp:public D3DApp
{
public:
	D11CubeMapApp( HINSTANCE hInstance );
	~D11CubeMapApp();

	bool			Init()override;

	void			UpdateScene( float dt )override;
	void			DrawScene()override;

	void			DrawScene(const Camera& cam, ID3D11ShaderResourceView* envTex);

private:

	void			BuildDynamicCubeMapViews();


	void			BuildCubeFaceCamera(float x, float y, float z);

private:
	CameraCtrlRotate		mRotateCameraController;

	SkyObject				mSky;
	StoneBuildingObject		mBuilding;
	ReflectSkullObject		mSkull;

	CommonEnvironment		mEnvironment;


	ID3D11DepthStencilView*			mDynamicCubeMapDSV;
	ID3D11RenderTargetView*			mDynamicCubeMapRTV[6];
	ID3D11ShaderResourceView*		mDynamicCubeMapSRV;

	D3D11_VIEWPORT					mCubeMapViewport;

	Camera							mCubeMapCamera[6];
};

#endif // ~_INSTANCINGDEMOAPP_H_