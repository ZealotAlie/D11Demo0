#ifndef _INSTANCINGDEMOAPP_H_
#define _INSTANCINGDEMOAPP_H_

#include "d3dApp.h"
#include "Effect.h"
#include "InstancedSkull.h"

class RenderObject;
class InstancingDemoApp:public D3DApp
{
public:
	InstancingDemoApp( HINSTANCE hInstance );
	~InstancingDemoApp();

	bool			Init()override;

	void			UpdateScene( float dt )override;
	void			DrawScene()override;

private:
	CommonEnvironment		mEnvironment;
	InstancedSkull			mSkulls;
	CameraCtrlRotate		mRotateCameraController;
};

#endif // ~_INSTANCINGDEMOAPP_H_