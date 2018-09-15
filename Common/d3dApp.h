//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Simple Direct3D demo application class.  
// Make sure you link: d3d11.lib d3dx11d.lib D3DCompiler.lib D3DX11EffectsD.lib 
//                     dxerr.lib dxgi.lib dxguid.lib.
// Link d3dx11.lib and D3DX11Effects.lib for release mode builds instead
//   of d3dx11d.lib and D3DX11EffectsD.lib.
//***************************************************************************************

#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dUtil.h"
#include "GameTimer.h"
#include <string>
#include "Camera.h"
#include <functional>
#include <vector>
#include "CameraCtrl.h"
#include <DirectXCollision.h>

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

typedef std::function<void(WPARAM btnState, int x, int y)> MouseDelegate;
class MouseEvent
{
public:
	void Add(const MouseDelegate& d)
	{
		mDelegates.push_back(d);
	}
	void Invoke(WPARAM btnState, int x, int y)
	{
		for(const auto&func :mDelegates)
		{
			func(btnState,x,y);
		}
	}
private:
	std::vector<MouseDelegate> mDelegates;
};

class IUnknownInstance;
class D3DApp
{
	typedef std::vector<IUnknownInstance*>  UnknownVector;
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();
	
	HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;

	static D3DApp*			GetInstance(){ return smInstance; }

	ID3D11Device*			GetDevice()	const{return md3dDevice;};
	ID3D11DeviceContext*	GetContext()const{return md3dImmediateContext;};

	int Run();
 
	// Framework methods.  Derived client class overrides these methods to 
	// implement specific application requirements.

	virtual bool		Init();
	virtual void		OnResize(); 
	virtual void		UpdateScene(float dt);
	virtual void		DrawScene(){}
	virtual LRESULT		MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	MouseEvent&			GetEventOnMouseDown(){return mEvtOnMouseDown;}
	MouseEvent&			GetEventOnMouseUp(){return mEvtOnMouseUp;}
	MouseEvent&			GetEventOnMouseMove(){return mEvtOnMouseMove;}

	Camera&				GetCamera(){return mCamera;}
	const Camera&		GetCamera()const{return mCamera;}
	void				SetCameraControll(CameraCtrlBase* pCtrl)
	{
		mCurCameraCtrl = pCtrl;
	}
	const BoundingFrustum& GetCameraFrustum()const{return mCameraFrustum;}

	RenderOptions		GetRenderOptions()const{ return mRenderOptions; }

	void				AddUnknownInstance( IUnknownInstance* pInstance ){ mInstanceUnknown.push_back( pInstance ); }

protected:
	bool				InitMainWindow();
	bool				InitDirect3D();

	void				CalculateFrameStats();

protected:
	static D3DApp*			smInstance;

	HINSTANCE				mhAppInst;
	HWND					mhMainWnd;
	bool					mAppPaused;
	bool					mMinimized;
	bool					mMaximized;
	bool					mResizing;
	UINT					m4xMsaaQuality;

	GameTimer				mTimer;

	ID3D11Device*			md3dDevice;
	ID3D11DeviceContext*	md3dImmediateContext;
	IDXGISwapChain*			mSwapChain;
	ID3D11Texture2D*		mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT			mScreenViewport;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring			mMainWndCaption;
	D3D_DRIVER_TYPE			md3dDriverType;
	int						mClientWidth;
	int						mClientHeight;
	bool					mEnable4xMsaa;

	MouseEvent				mEvtOnMouseDown;
	MouseEvent				mEvtOnMouseUp;
	MouseEvent				mEvtOnMouseMove;

	Camera					mCamera;
	BoundingFrustum			mCameraFrustum;

	CameraCtrlBase*			mCurCameraCtrl;
	RenderOptions			mRenderOptions;
	UnknownVector			mInstanceUnknown;
};

#endif // D3DAPP_H