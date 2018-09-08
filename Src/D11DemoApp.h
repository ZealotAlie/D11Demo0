#ifndef _D11DEMOAPP_H_
#define _D11DEMOAPP_H_

#include "d3dApp.h"
#include "BlurFilter.h"
#include "RenderObjects.h"

class RenderObject;
class D11DemoApp:public D3DApp
{
public:
	D11DemoApp(HINSTANCE hInstance);
	~D11DemoApp();

	bool						Init()override;
	void						OnResize()override;

	void						UpdateScene(float dt)override;
	void						DrawScene()override;
	DirectionalLight const*		GetLights()const{return mDirLights;}

private:
	void						BuildOffScreenViews();

private:
	CameraCtrlRotate			mRotateCameraController;
	std::vector<RenderObject*>	mRenderObjects;

	DirectionalLight			mDirLights[3];
	BlurFilter					mBlurFilter;
	int							mBlurCount;

	FullScreenQuadObject		mScreenQuad;

	ID3D11ShaderResourceView*	mOffscreenSRV;
	ID3D11UnorderedAccessView*	mOffscreenUAV;
	ID3D11RenderTargetView*		mOffscreenRTV;
};

#endif