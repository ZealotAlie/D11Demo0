#include "D11CubeMapApp.h"
#include "UnknownInstance\Pos.h"
#include "UnknownInstance\Basic32.h"

D11CubeMapApp::D11CubeMapApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	,mRotateCameraController(*this,10,100)
{
	mMainWndCaption = L"Direct 11 Instancing App";
	mRenderOptions = Lighting;
	SetCameraControll(&mRotateCameraController);

	mEnvironment.dirLights[0].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mEnvironment.dirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mEnvironment.dirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mEnvironment.dirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mEnvironment.dirLights[1].Diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mEnvironment.dirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mEnvironment.dirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mEnvironment.dirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mEnvironment.dirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mEnvironment.dirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
}

D11CubeMapApp::~D11CubeMapApp()
{

}

bool D11CubeMapApp::Init()
{
	if( !D3DApp::Init() )
	{
		return false;
	}

	std::vector<std::wstring> fileNames;
	fileNames.push_back(L"Textures/grasscube1024.dds");
	mSky.Build( this, fileNames, 5000.0f );
	mBuilding.Build( this );
	mSkull.Build( this );

	AddUnknownInstance(new SkyEffect( md3dDevice,L"FX/Sky.fx"));
	AddUnknownInstance(new BasicEffect( md3dDevice, L"FX/Basic.fx"));

	mEnvironment.Apply(BasicEffect::Ptr());

	BuildDynamicCubeMapViews();
	auto const& skullWorld = mSkull.GetWorld();
	BuildCubeFaceCamera(skullWorld._41,skullWorld._42,skullWorld._43);
	return true;
}

void D11CubeMapApp::UpdateScene(float dt)
{
	D3DApp::UpdateScene(dt);
}

void D11CubeMapApp::DrawScene()
{
	Camera currentCamera;
	memcpy( &currentCamera, &mCamera, sizeof( mCamera ) );

	ID3D11RenderTargetView* renderTargets[1];

	md3dImmediateContext->RSSetViewports(1, &mCubeMapViewport);
	for( int i =0; i < 6; ++i )
	{
		renderTargets[0] = mDynamicCubeMapRTV[i];

		md3dImmediateContext->ClearDepthStencilView(mDynamicCubeMapDSV,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);
		md3dImmediateContext->ClearRenderTargetView(renderTargets[0],reinterpret_cast<const float*>(&Colors::LightSteelBlue));

		md3dImmediateContext->OMSetRenderTargets( 1, renderTargets, mDynamicCubeMapDSV );

		DrawScene( mCubeMapCamera[i], nullptr );
	}

	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	renderTargets[0] = mRenderTargetView;
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

	// Have hardware generate lower mipmap levels of cube map.
	md3dImmediateContext->GenerateMips(mDynamicCubeMapSRV);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView,reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);

	DrawScene( currentCamera, mDynamicCubeMapSRV );

	HR(mSwapChain->Present(0, 0));
}

void D11CubeMapApp::DrawScene(const Camera& cam, ID3D11ShaderResourceView* envTex)
{
	memcpy( &mCamera, &cam, sizeof( mCamera ) );

	mSky.Draw(this);

	BasicEffect::Ptr()->SetCubeMap(mSky.GetTextureSRV());
	mBuilding.Draw(this);

	if(envTex)
	{
		BasicEffect::Ptr()->SetCubeMap( envTex );
		mSkull.Draw( this );
		BasicEffect::Ptr()->SetCubeMap( nullptr );
		BasicEffect::Ptr()->Light3TexReflectTech->GetPassByIndex(0)->Apply(0,md3dImmediateContext);
	}
}

void D11CubeMapApp::BuildDynamicCubeMapViews()
{
	const int CubeMapSize = 256;

	//
	// Cubemap is a special texture array with 6 elements.
	//

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = CubeMapSize;
	texDesc.Height = CubeMapSize;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* cubeTex = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &cubeTex));

	//
	// Create a render target view to each cube map face 
	// (i.e., each element in the texture array).
	// 

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for(int i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HR(md3dDevice->CreateRenderTargetView(cubeTex, &rtvDesc, &mDynamicCubeMapRTV[i]));
	}

	//
	// Create a shader resource view to the cube map.
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	HR(md3dDevice->CreateShaderResourceView(cubeTex, &srvDesc, &mDynamicCubeMapSRV));

	ReleaseCOM(cubeTex);

	//
	// We need a depth texture for rendering the scene into the cubemap
	// that has the same resolution as the cubemap faces.  
	//

	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = CubeMapSize;
	depthTexDesc.Height = CubeMapSize;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	HR(md3dDevice->CreateTexture2D(&depthTexDesc, 0, &depthTex));

	// Create the depth stencil view for the entire cube
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags  = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(md3dDevice->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSV));

	ReleaseCOM(depthTex);

	//
	// Viewport for drawing into cubemap.
	// 

	mCubeMapViewport.TopLeftX = 0.0f;
	mCubeMapViewport.TopLeftY = 0.0f;
	mCubeMapViewport.Width    = (float)CubeMapSize;
	mCubeMapViewport.Height   = (float)CubeMapSize;
	mCubeMapViewport.MinDepth = 0.0f;
	mCubeMapViewport.MaxDepth = 1.0f;
}

void D11CubeMapApp::BuildCubeFaceCamera(float x, float y, float z)
{
	// Generate the cube map about the given position.
	XMFLOAT3 center(x, y, z);
	XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

	// Look along each coordinate axis.
	XMFLOAT3 targets[6] = 
	{
		XMFLOAT3(x+1.0f, y, z), // +X
		XMFLOAT3(x-1.0f, y, z), // -X
		XMFLOAT3(x, y+1.0f, z), // +Y
		XMFLOAT3(x, y-1.0f, z), // -Y
		XMFLOAT3(x, y, z+1.0f), // +Z
		XMFLOAT3(x, y, z-1.0f)  // -Z
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	XMFLOAT3 ups[6] = 
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
	};

	for(int i = 0; i < 6; ++i)
	{
		mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
		mCubeMapCamera[i].SetLens(0.5f*XM_PI, 1.0f, 0.1f, 1000.0f);
		mCubeMapCamera[i].UpdateViewMatrix();
	}
}