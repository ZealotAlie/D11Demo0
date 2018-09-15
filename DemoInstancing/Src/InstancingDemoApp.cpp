#include "InstancingDemoApp.h"
#include "InstancingEffect.h"
#include "InstancedData.h"
#include "UnknownInstance\Basic32.h"

InstancingDemoApp::InstancingDemoApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	,mRotateCameraController(*this,10,800)
{
	mMainWndCaption = L"Direct 11 Instancing App";
	mRenderOptions = Lighting;
	SetCameraControll(&mRotateCameraController);
}

InstancingDemoApp::~InstancingDemoApp()
{

}

bool InstancingDemoApp::Init()
{
	if( !D3DApp::Init() )
	{
		return false;
	}

	this->AddUnknownInstance( new InstancingEffect( md3dDevice, L"FX/InstancedBasic.fx" ) );
	this->AddUnknownInstance( new BasicEffect( md3dDevice, L"FX/Basic.fx" ) );
	mEnvironment.Apply( InstancingEffect::Ptr() );
	mEnvironment.Apply( BasicEffect::Ptr() );

	mSkulls.Build(this);

	return true;
}

void InstancingDemoApp::UpdateScene(float dt)
{
	D3DApp::UpdateScene(dt);

	if( GetAsyncKeyState('1') & 0x8000 )
		mSkulls.SetFrustumCullingType( CullingNone );

	if( GetAsyncKeyState('2') & 0x8000 )
		mSkulls.SetFrustumCullingType( LocalFrustumCulling );

	if( GetAsyncKeyState('3') & 0x8000 )
		mSkulls.SetFrustumCullingType( CameraFrustumCulling );

	mSkulls.Update(dt);


	std::wostringstream outs;   
	outs.precision(6);
	outs << L"Instancing and Culling Demo" << 
		L"    " << mSkulls.GetVisibleObjectCount();
	mMainWndCaption = outs.str();
}

void InstancingDemoApp::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets( 1, renderTargets, mDepthStencilView );

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView,reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);

	mSkulls.Draw(this);

	HR(mSwapChain->Present(0, 0));
}