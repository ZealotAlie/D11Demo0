#include "D11DemoApp.h"
#include "DemoEffects.h"
#include "RenderObjects.h"
#include "UnknownInstance/RenderStates.h"
#include "UnknownInstance/Basic32.h"

template<typename T>
static void SetupLightFog(T* pEffect, const DirectionalLight* pLights, const FXMVECTOR fogColor, float fogStart, float fogRange)
{
	pEffect->SetDirLights(pLights);
	pEffect->SetFogColor(fogColor);
	pEffect->SetFogStart(fogStart);
	pEffect->SetFogRange(fogRange);
}

D11DemoApp::D11DemoApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	,mRotateCameraController(*this,10,800)
	,mOffscreenSRV(nullptr)
	,mOffscreenUAV(nullptr)
	,mOffscreenRTV(nullptr)
	,mBlurCount(0)
{
	mMainWndCaption = L"Direct 11 Demo App";

	SetCameraControll(&mRotateCameraController);

	mDirLights[0].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.4f, -0.82462f, 0.4f);

	mDirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	auto wall = new WallObject();
	mRenderObjects.push_back(new MountObject());
	mRenderObjects.push_back(new SkullObject( wall ));
	mRenderObjects.push_back(new TreeObject());
	mRenderObjects.push_back(wall);
	mRenderObjects.push_back(new WaterObject());
	//mRenderObjects.push_back(new CsWaterObject());
}

D11DemoApp::~D11DemoApp()
{
	for(auto pObj :mRenderObjects)
	{
		delete pObj;
	}
	mRenderObjects.clear();
	ReleaseCOM(mOffscreenSRV);
	ReleaseCOM(mOffscreenUAV);
	ReleaseCOM(mOffscreenRTV);
}

bool D11DemoApp::Init()
{
	if(!D3DApp::Init())
	{
		return false;
	}

	AddUnknownInstance( new BasicEffect( md3dDevice, L"FX/Basic.fx" )			);
	AddUnknownInstance( new TreeSpriteEffect( md3dDevice,L"FX/TreeSprite.fx" )	);
	AddUnknownInstance( new BlurEffect( md3dDevice,L"FX/Blur.fx" )				);
	AddUnknownInstance( new WaterEffect( md3dDevice,L"FX/Water.fx" )			);

	for(auto pObj :mRenderObjects)
	{
		pObj->Build(this);
	}
	mScreenQuad.Build(this);

	auto fogColor = Colors::Gray;
	const float fogStart = 25;
	const float fogRange = 400;

	SetupLightFog( BasicEffect::Ptr(),			mDirLights, fogColor, fogStart, fogRange );
	SetupLightFog( TreeSpriteEffect::Ptr(),		mDirLights, fogColor, fogStart, fogRange );
	SetupLightFog( WaterEffect::Ptr(),			mDirLights, fogColor, fogStart, fogRange );

	BuildOffScreenViews();
	return true;
}

void D11DemoApp::OnResize()
{
	D3DApp::OnResize();
	BuildOffScreenViews();
}

void D11DemoApp::UpdateScene(float dt)
{
	D3DApp::UpdateScene(dt);
	for(auto pObj :mRenderObjects)
	{
		pObj->Update(dt);
	}

	if( GetAsyncKeyState(VK_UP) & 0x0001 )
		++mBlurCount;
	if( GetAsyncKeyState(VK_DOWN) & 0x0001 )
		--mBlurCount;
	mBlurCount = MathHelper::Max( mBlurCount, 0 );
}

void D11DemoApp::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1] = { mOffscreenRTV };
	md3dImmediateContext->OMSetRenderTargets( 1, renderTargets, mDepthStencilView );

	md3dImmediateContext->ClearRenderTargetView(mOffscreenRTV,reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);

	//Draw
	for(auto pObj :mRenderObjects)
	{
		pObj->Draw( this );
	}
	//Draw Transparency
	for(auto pObj :mRenderObjects)
	{
		pObj->DrawTransparency( this );
	}

	//Blur
	renderTargets[0] = mRenderTargetView;
	md3dImmediateContext->OMSetRenderTargets( 1, renderTargets, mDepthStencilView );
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	if( mBlurCount == 0 )
	{
		mScreenQuad.DrawQuad(this,mOffscreenSRV);
		BasicEffect::Ptr()->SetDiffuseMap(nullptr);
		BasicEffect::Ptr()->Light0TexTech->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
	}
	else
	{
		mBlurFilter.BlurInPlace(md3dImmediateContext,mOffscreenSRV,mOffscreenUAV,mBlurCount);
		mScreenQuad.DrawQuad(this,mBlurFilter.GetBlurredOutput());
	}

	HR(mSwapChain->Present(0,0));
}

void D11DemoApp::BuildOffScreenViews()
{
	ReleaseCOM(mOffscreenSRV);
	ReleaseCOM(mOffscreenRTV);
	ReleaseCOM(mOffscreenUAV);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mClientWidth;
	texDesc.Height = mClientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* offScreenTex = nullptr;
	HR(md3dDevice->CreateTexture2D(&texDesc,0,&offScreenTex));

	HR(md3dDevice->CreateShaderResourceView(offScreenTex, 0, &mOffscreenSRV));
	HR(md3dDevice->CreateRenderTargetView(offScreenTex, 0, &mOffscreenRTV));
	HR(md3dDevice->CreateUnorderedAccessView(offScreenTex, 0, &mOffscreenUAV));

	// View saves a reference to the texture so we can release our reference.
	ReleaseCOM(offScreenTex);


	mBlurFilter.Init(md3dDevice, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
}