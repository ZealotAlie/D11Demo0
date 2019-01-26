#include "NDSsaoApp.h"

#include <windows.h>
#include <crtdbg.h>

#include "UnknownInstance/Pos.h"
#include "SsaoEffect.h"
#include "SceneObject.h"
#include "GeometryGenerator.h"
#include "BuildShadowMapEffect.h"
#include "UnknownInstance/RenderStates.h"
#include "DebugTexEffect.h"

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	NDSsaoApp myApp(hInstance);

	if(!myApp.Init())
	{
		return 0;
	}

	return myApp.Run();
}

NDSsaoApp::NDSsaoApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
	, mRotateCameraController(*this,1,200)
	, mSmap( 2048, 2048 )
	, mLightRotationAngle( 0 )
	, mDrawOptions( RenderOptionsBasic )
{
	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
	SetCameraControll(&mRotateCameraController);

	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);

	mEnvironment.dirLights[0].Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mEnvironment.dirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.4f, 1.0f);
	mEnvironment.dirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mEnvironment.dirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mEnvironment.dirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mEnvironment.dirLights[1].Diffuse  = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	mEnvironment.dirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mEnvironment.dirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mEnvironment.dirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mEnvironment.dirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);


	mOriginalLightDir[0] = mEnvironment.dirLights[0].Direction;
	mOriginalLightDir[1] = mEnvironment.dirLights[1].Direction;
	mOriginalLightDir[2] = mEnvironment.dirLights[2].Direction;

}

NDSsaoApp::~NDSsaoApp()
{

}

bool NDSsaoApp::Init()
{
	if(!D3DApp::Init())
	{
		return false;
	}
	mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	AddUnknownInstance(new SkyEffect( md3dDevice,				L"FX/Sky.fx"));
	AddUnknownInstance(new SsaoEffect( md3dDevice,				L"FX/Ssao.fx"));
	AddUnknownInstance(new SsaoBlurEffect( md3dDevice,			L"FX/SsaoBlur.fx"));
	AddUnknownInstance(new BasicEffect( md3dDevice,				L"FX/Basic.fx"));
	AddUnknownInstance(new NormalMapEffect(md3dDevice,			L"FX/NormalMap.fx"));
	AddUnknownInstance(new DisplacementMapEffect( md3dDevice,	L"FX/DisplacementMap.fx"));
	AddUnknownInstance(new BuildShadowMapEffect( md3dDevice,	L"FX/BuildShadowMap.fx"));
	AddUnknownInstance(new SsaoNormalDepthEffect( md3dDevice,	L"FX/SsaoNormalDepth.fx"));
	AddUnknownInstance(new DebugTexEffect( md3dDevice,			L"FX/DebugTexture.fx"));

	mEnvironment.Apply(NormalMapEffect::Ptr());
	mEnvironment.Apply(BasicEffect::Ptr());

	std::vector<std::wstring> fileNames;
	fileNames.push_back(L"Textures/desertcube1024.dds");
	mSky.Build( this, fileNames, 5000.0f );

	mSsao.Build(this);
	mSmap.Build(md3dDevice);

	BuildShapeGeometryBuffers();
	BuildSkullGeometryBuffers();
	BuildScreenQuadGeometryBuffers();

	return true;
}

void NDSsaoApp::DrawScene()
{
	//Draw shadow map
	mSmap.BindDsvAndSetNullRenderTarget(md3dImmediateContext);
	DrawSceneToShadowMap();
	md3dImmediateContext->RSSetState(0);

	//Draw SSAO normal depth MAP
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	mSsao.SetNormalDepthRenderTarget(md3dImmediateContext,mDepthStencilView);
	DrawSceneToSsaoNormalDepthMap();

	//Now compute the ambient occlusion.
	mSsao.ComputeSsao(this);
	mSsao.BlurAmbientMap(4);

	//Draw normal scenes
	ID3D11RenderTargetView* renderTargets[1] = {mRenderTargetView};
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Debug view SSAO map.
	if( GetAsyncKeyState('Z') & 0x8000 )
	{
		DrawScreenQuad(mSsao.AmbientSRV());
	}

	//SKY
	mSky.Draw(this);

	// Set per frame constants.
	BasicEffect::Ptr()->SetDirLights(mEnvironment.dirLights);
	BasicEffect::Ptr()->SetEyePosW(mCamera.GetPosition());
	BasicEffect::Ptr()->SetCubeMap(mSky.GetTextureSRV());
	BasicEffect::Ptr()->SetShadowMap(mSmap.DepthMapSRV());
	BasicEffect::Ptr()->SetSsaoMap(mSsao.AmbientSRV());

	NormalMapEffect::Ptr()->SetDirLights(mEnvironment.dirLights);
	NormalMapEffect::Ptr()->SetEyePosW(mCamera.GetPosition());
	NormalMapEffect::Ptr()->SetCubeMap(mSky.GetTextureSRV());
	NormalMapEffect::Ptr()->SetShadowMap(mSmap.DepthMapSRV());
	NormalMapEffect::Ptr()->SetSsaoMap(mSsao.AmbientSRV());

	DisplacementMapEffect::Ptr()->SetDirLights(mEnvironment.dirLights);
	DisplacementMapEffect::Ptr()->SetEyePosW(mCamera.GetPosition());
	DisplacementMapEffect::Ptr()->SetCubeMap(mSky.GetTextureSRV());
	DisplacementMapEffect::Ptr()->SetShadowMap(mSmap.DepthMapSRV());
	// These properties could be set per object if needed.
	DisplacementMapEffect::Ptr()->SetHeightScale(0.07f);
	DisplacementMapEffect::Ptr()->SetMaxTessDistance(1.0f);
	DisplacementMapEffect::Ptr()->SetMinTessDistance(25.0f);
	DisplacementMapEffect::Ptr()->SetMinTessFactor(1.0f);
	DisplacementMapEffect::Ptr()->SetMaxTessFactor(5.0f);

	mShapesVertexState.Apply(md3dImmediateContext);

	if( mDrawOptions == RenderOptionsDisplacementMap )
	{
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else
	{
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	//Box
	DrawWithOption(mDrawOptions, mBox, false);
	//Grid
	DrawWithOption(mDrawOptions, mGrid, false);

	for (int i =0;i<10;++i)
	{
		//Cylinders
		DrawWithOption(mDrawOptions, mCylinders[i], false);
	}

	// FX sets tessellation stages, but it does not disable them.  So do that here to turn off tessellation.
	md3dImmediateContext->HSSetShader(0, 0, 0);
	md3dImmediateContext->DSSetShader(0, 0, 0);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (int i =0;i<10;++i)
	{
		//Spheres
		DrawWithOption(RenderOptionsBasic, mSphere[i], true);
	}

	//Skull
	mSkullVertexState.Apply(md3dImmediateContext);
	DrawWithOption(RenderOptionsBasic, mSkull, true);

	// restore default states.
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);
	md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff); 

	// Unbind shadow map and AmbientMap as a shader input because we are going to render
	// to it next frame.  These textures can be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	md3dImmediateContext->PSSetShaderResources(0, 16, nullSRV);

	HR(mSwapChain->Present(0, 0));
}

void NDSsaoApp::UpdateScene(float dt)
{
	D3DApp::UpdateScene(dt);

	if( GetAsyncKeyState('1') & 0x8000 )
		mDrawOptions = RenderOptionsBasic;

	if( GetAsyncKeyState('2') & 0x8000 )
		mDrawOptions = RenderOptionsNormalMap;

	if( GetAsyncKeyState('3') & 0x8000 )
		mDrawOptions = RenderOptionsDisplacementMap;

	mLightRotationAngle += 0.1f*dt;

	XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);
	for(int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&mOriginalLightDir[i]);
		lightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&mEnvironment.dirLights[i].Direction, lightDir);
	}

	BuildShadowTransform();
}

void NDSsaoApp::OnResize()
{
	D3DApp::OnResize();

	mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mSsao.OnSize(this);
}

void NDSsaoApp::BuildShadowTransform()
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&mEnvironment.dirLights[0].Direction);
	XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = V*P*T;

	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S);
}

#pragma region
void NDSsaoApp::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 50, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	int boxVertexOffset      = 0;
	int gridVertexOffset     = boxVertexOffset + box.Vertices.size();
	int sphereVertexOffset   = gridVertexOffset + grid.Vertices.size();
	int cylinderVertexOffset = sphereVertexOffset + sphere.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	int boxIndexOffset      = 0;
	int gridIndexOffset     = box.Indices.size();
	int sphereIndexOffset   = gridIndexOffset + grid.Indices.size();
	int cylinderIndexOffset = sphereIndexOffset + sphere.Indices.size();

	mBox.SetBufferOffset(box.Indices.size(),boxIndexOffset,boxVertexOffset);
	mGrid.SetBufferOffset(grid.Indices.size(),gridIndexOffset,gridVertexOffset);
	for(int i=0; i<ARRAYSIZE(mCylinders); ++i)
	{
		mSphere[i].SetBufferOffset(sphere.Indices.size(),sphereIndexOffset,sphereVertexOffset);
		mCylinders[i].SetBufferOffset(cylinder.Indices.size(),cylinderIndexOffset,cylinderVertexOffset);
	}

	UINT totalVertexCount = 
		box.Vertices.size() + 
		grid.Vertices.size() + 
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount = 
		box.Indices.size() + 
		grid.Indices.size() + 
		sphere.Indices.size() +
		cylinder.Indices.size();

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<PosNormalTexTan> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = box.Vertices[i].Position;
		vertices[k].Normal   = box.Vertices[i].Normal;
		vertices[k].Tex      = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;
	}

	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = grid.Vertices[i].Position;
		vertices[k].Normal   = grid.Vertices[i].Normal;
		vertices[k].Tex      = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;
	}

	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = sphere.Vertices[i].Position;
		vertices[k].Normal   = sphere.Vertices[i].Normal;
		vertices[k].Tex      = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;
	}

	for(size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = cylinder.Vertices[i].Position;
		vertices[k].Normal   = cylinder.Vertices[i].Normal;
		vertices[k].Tex      = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(PosNormalTexTan) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, mShapesVertexState.GetAddressOfVB()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, mShapesVertexState.GetAddressOfIB()));

	//////////////////////////////////////////////////////////////////////////
	ID3D11ShaderResourceView* stoneTextSRV;
	HR(CreateDDSTextureFromFile(md3dDevice, 
		L"Textures/floor.dds", nullptr, &stoneTextSRV ));

	ID3D11ShaderResourceView* brickTexSRV;
	HR(CreateDDSTextureFromFile(md3dDevice, 
		L"Textures/bricks.dds", nullptr, &brickTexSRV ));

	ID3D11ShaderResourceView* stoneNormalTexSRV;
	HR(CreateDDSTextureFromFile(md3dDevice, 
		L"Textures/floor_nmap.dds", nullptr, &stoneNormalTexSRV ));

	ID3D11ShaderResourceView* brickNormalTexSRV;
	HR(CreateDDSTextureFromFile(md3dDevice, 
		L"Textures/bricks_nmap.dds", nullptr, &brickNormalTexSRV ));

	//Box
	XMStoreFloat4x4(&mBox.GetWorldTranform(), 
		XMMatrixMultiply(XMMatrixScaling(3.0f, 1.0f, 3.0f), XMMatrixTranslation(0.0f, 0.5f, 0.0f)));
	XMStoreFloat4x4(&mBox.GetTexTransform(), XMMatrixScaling(2.0f, 1.0f, 1.0f));
	mBox.GetMaterial().Ambient  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mBox.GetMaterial().Diffuse  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mBox.GetMaterial().Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBox.GetMaterial().Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mBox.SetTexture(brickTexSRV);
	mBox.SetNormalTexture(brickNormalTexSRV);
	mSceneObjects.push_back(&mBox);

	//Grid
	XMStoreFloat4x4(&mGrid.GetWorldTranform(), XMMatrixIdentity());
	XMStoreFloat4x4(&mGrid.GetTexTransform(), XMMatrixScaling(8.0f, 10.0f, 1.0f));
	mGrid.GetMaterial().Ambient  = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mGrid.GetMaterial().Diffuse  = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mGrid.GetMaterial().Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mGrid.GetMaterial().Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mGrid.SetTexture(stoneTextSRV);
	mGrid.SetNormalTexture(stoneNormalTexSRV);
	mSceneObjects.push_back(&mGrid);

	//cylinders
	for(int i = 0; i < 10; ++i)
	{
		float x = (i%2) == 0? -5.0f : +5.0f;
		float z = (i/2) * 5.0f - 10.0f;
		XMStoreFloat4x4(&mCylinders[i].GetWorldTranform(),XMMatrixTranslation(x, 1.5f, z));
		XMStoreFloat4x4(&mSphere[i].GetWorldTranform(),XMMatrixTranslation(x, 3.5f, z));

		XMStoreFloat4x4(&mCylinders[i].GetTexTransform(),XMMatrixScaling(1.0f, 2.0f, 1.0f));
		mCylinders[i].GetMaterial().Ambient  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		mCylinders[i].GetMaterial().Diffuse  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		mCylinders[i].GetMaterial().Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
		mCylinders[i].GetMaterial().Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mCylinders[i].SetTexture(brickTexSRV);
		mCylinders[i].SetNormalTexture(brickNormalTexSRV);
		mSceneObjects.push_back(&mCylinders[i]);

		XMStoreFloat4x4(&mSphere[i].GetTexTransform(),XMMatrixIdentity());
		mSphere[i].GetMaterial().Ambient  = XMFLOAT4(0.3f, 0.4f, 0.5f, 1.0f);
		mSphere[i].GetMaterial().Diffuse  = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
		mSphere[i].GetMaterial().Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
		mSphere[i].GetMaterial().Reflect  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		mSceneObjects.push_back(&mSphere[i]);
	}

	ReleaseCOM(stoneTextSRV);
	ReleaseCOM(brickTexSRV);
	ReleaseCOM(stoneNormalTexSRV);
	ReleaseCOM(brickNormalTexSRV);
}

void NDSsaoApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if(!fin)
	{
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Basic32> vertices(vcount);
	for(UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	int skullIndexCount = 3*tcount;
	std::vector<UINT> indices(skullIndexCount);
	for(UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, mSkullVertexState.GetAddressOfVB()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * skullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, mSkullVertexState.GetAddressOfIB()));

	mSkull.SetBufferOffset(skullIndexCount, 0, 0);
	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkull.GetWorldTranform(), XMMatrixMultiply(skullScale, skullOffset));
	mSkull.GetMaterial().Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkull.GetMaterial().Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkull.GetMaterial().Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 16.0f);
	mSkull.GetMaterial().Reflect  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
}

void NDSsaoApp::BuildScreenQuadGeometryBuffers()
{
	GeometryGenerator::MeshData quad;

	GeometryGenerator geoGen;
	geoGen.CreateFullscreenQuad(quad);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Basic32> vertices(quad.Vertices.size());

	for(UINT i = 0; i < quad.Vertices.size(); ++i)
	{
		vertices[i].Pos    = quad.Vertices[i].Position;
		vertices[i].Normal = quad.Vertices[i].Normal;
		vertices[i].Tex    = quad.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Basic32) * quad.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, mScreenVertexState.GetAddressOfVB()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &quad.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, mScreenVertexState.GetAddressOfIB()));
}

#pragma endregion

void NDSsaoApp::DrawSceneToShadowMap()
{
	BuildShadowMapEffect::Ptr()->SetEyePosW(mCamera.GetPosition());

	// These properties could be set per object if needed.
	BuildShadowMapEffect::Ptr()->SetHeightScale(0.07f);
	BuildShadowMapEffect::Ptr()->SetMaxTessDistance(1.0f);
	BuildShadowMapEffect::Ptr()->SetMinTessDistance(25.0f);
	BuildShadowMapEffect::Ptr()->SetMinTessFactor(1.0f);
	BuildShadowMapEffect::Ptr()->SetMaxTessFactor(5.0f);

	mShapesVertexState.Apply(md3dImmediateContext);
	if(mDrawOptions != RenderOptionsDisplacementMap)
	{
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for(size_t i =0;i<mSceneObjects.size();++i)
		{
			mSceneObjects[i]->Draw(this,BuildShadowMapEffect::Ptr(),BuildShadowMapEffect::Ptr()->BuildShadowMapTech, 
				&mLightView, &mLightProj);
		}
	}
	else
	{
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		for(size_t i =0;i<mSceneObjects.size();++i)
		{
			mSceneObjects[i]->Draw(this,BuildShadowMapEffect::Ptr(),BuildShadowMapEffect::Ptr()->TessBuildShadowMapTech, 
				&mLightView, &mLightProj);
		}
	}


	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	md3dImmediateContext->HSSetShader(0, 0, 0);
	md3dImmediateContext->DSSetShader(0, 0, 0);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	mSkullVertexState.Apply(md3dImmediateContext);
	mSkull.Draw(this,BuildShadowMapEffect::Ptr(),BuildShadowMapEffect::Ptr()->BuildShadowMapTech, &mLightView, &mLightProj);
}

void NDSsaoApp::DrawSceneToSsaoNormalDepthMap()
{
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mShapesVertexState.Apply(md3dImmediateContext);
	for(size_t i =0;i<mSceneObjects.size();++i)
	{
		mSceneObjects[i]->Draw(this,SsaoNormalDepthEffect::Ptr(),SsaoNormalDepthEffect::Ptr()->NormalDepthTech);
	}

	md3dImmediateContext->RSSetState(0);

	mSkullVertexState.Apply(md3dImmediateContext);
	mSkull.Draw(this,SsaoNormalDepthEffect::Ptr(),SsaoNormalDepthEffect::Ptr()->NormalDepthTech);
}

void NDSsaoApp::DrawScreenQuad(ID3D11ShaderResourceView* srv)
{
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mScreenVertexState.Apply(md3dImmediateContext);

	// Scale and shift quad to lower-right corner.
	XMMATRIX world(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f);

	ID3DX11EffectTechnique* tech = DebugTexEffect::Ptr()->ViewRedTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		DebugTexEffect::Ptr()->SetWorldViewProj(world);
		DebugTexEffect::Ptr()->SetTexture(srv);

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}
}

void NDSsaoApp::DrawWithOption(EDrawOptions option,SceneObject& obj, bool drawReflect)
{
	switch (option)
	{
	case RenderOptionsBasic:
		XMStoreFloat4x4(&obj.GetShadowTransform(),
			XMLoadFloat4x4(&mShadowTransform)*XMLoadFloat4x4(&obj.GetWorldTranform()));
		obj.Draw(this,BasicEffect::Ptr(), drawReflect ? BasicEffect::Ptr()->Light3ReflectTech :BasicEffect::Ptr()->Light3TexTech);
		break;

	case RenderOptionsNormalMap:
		XMStoreFloat4x4(&obj.GetShadowTransform(),
			XMLoadFloat4x4(&mShadowTransform)*XMLoadFloat4x4(&obj.GetWorldTranform()));
		obj.Draw(this,NormalMapEffect::Ptr(),drawReflect ? NormalMapEffect::Ptr()->Light3ReflectTech :NormalMapEffect::Ptr()->Light3TexTech);
		break;

	case RenderOptionsDisplacementMap:
		obj.GetShadowTransform() = mShadowTransform;
		obj.Draw(this,DisplacementMapEffect::Ptr(),drawReflect ? DisplacementMapEffect::Ptr()->Light3ReflectTech :DisplacementMapEffect::Ptr()->Light3TexTech);
		break;
	default:
		break;
	}
}