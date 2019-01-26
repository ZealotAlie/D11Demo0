#include "TPCApp.h"
#include "UnknownInstance\Pos.h"
#include "UnknownInstance\RenderStates.h"
#include "TerrainEffect.h"
#include "ParticleEffect.h"

TPCApp::TPCApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
	, mRotateCameraController(*this,10,1000)
	, mCameraCtrlFPS(&mTerrain)
	, mFlareTexSRV(nullptr)
	, mRainTexSRV(nullptr)
	, mRandomTexSRV(nullptr)
{
	mEnable4xMsaa = true;
	SetCameraControll(&mCameraCtrlFPS);
	mMainWndCaption = L"Terrain Particle Cube Demo";
}

TPCApp::~TPCApp()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mFlareTexSRV);
	ReleaseCOM(mRainTexSRV);
	ReleaseCOM(mRandomTexSRV);
}

bool TPCApp::Init()
{
	if( !D3DApp::Init() )
	{
		return false;
	}

	AddUnknownInstance(new SkyEffect( md3dDevice,L"FX/Sky.fx"));
	AddUnknownInstance(new TerrainEffect( md3dDevice, L"FX/Terrain.fx"));
	AddUnknownInstance(new RainEffect( md3dDevice,L"FX/Rain.fx"));
	AddUnknownInstance(new FireEffect( md3dDevice,L"FX/Fire.fx"));

	std::vector<std::wstring> fileNames;
	fileNames.push_back(L"Textures/grasscube1024.dds");
	mSky.Build( this, fileNames, 5000.0f );

	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/terrain.raw";
	tii.LayerMapFilename0 = L"Textures/grass.dds";
	tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"Textures/stone.dds";
	tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"Textures/snow.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;
	mTerrain.Init(md3dDevice, md3dImmediateContext, tii);

	mRandomTexSRV = d3dHelper::CreateRandomTexture1DSRV(md3dDevice);
	std::vector<std::wstring> flares;
	flares.push_back(L"Textures\\flare0.dds");

	mFlareTexSRV = d3dHelper::CreateTexture2DArraySRV(md3dDevice, md3dImmediateContext, flares);
	mFire.Init(md3dDevice, FireEffect::Ptr(), mFlareTexSRV, mRandomTexSRV, 500); 
	mFire.SetEmitPos(XMFLOAT3(0.0f, 1.0f, 120.0f));

	std::vector<std::wstring> raindrops;
	raindrops.push_back(L"Textures\\raindrop.dds");
	mRainTexSRV = d3dHelper::CreateTexture2DArraySRV(md3dDevice, md3dImmediateContext, raindrops);
	mRain.Init(md3dDevice, RainEffect::Ptr(), mRainTexSRV, mRandomTexSRV, 10000); 

	return true;
}

void TPCApp::DrawScene()
{
	D3DApp::DrawScene();

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	mSky.Draw(this);

	mTerrain.Draw(md3dImmediateContext, mCamera, mEnvironment.dirLights);

	// Draw particle systems last so it is blended with scene.
	mFire.SetEyePos(mCamera.GetPosition());
	mFire.Draw(md3dImmediateContext, mCamera);
	md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff); // restore default

	mRain.SetEyePos(mCamera.GetPosition());
	mRain.SetEmitPos(mCamera.GetPosition());
	mRain.Draw(md3dImmediateContext, mCamera);


	// restore default states.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);
	md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff); 

	HR(mSwapChain->Present(0, 0));
}

void TPCApp::UpdateScene(float dt)
{
	D3DApp::UpdateScene(dt);

	mFire.Update(dt, mTimer.TotalTime());
	mRain.Update(dt, mTimer.TotalTime());
}