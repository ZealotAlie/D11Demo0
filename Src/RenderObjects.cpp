#include "RenderObjects.h"
#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include "DemoEffects.h"
#include "UnknownInstance/RenderStates.h"
#include "UnknownInstance/Basic32.h"
#include "D11DemoApp.h"
#include "TreePointSprite.h"

const float LAND_SIZE = 160.f;

struct MountHelper
{
	static float GetHeight( float x, float z )
	{
		return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
	}
	static XMFLOAT3 GetNormal( float x, float z )
	{
		// n = (-df/dx, 1, -df/dz)
		XMFLOAT3 n(
			-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
			1.0f,
			-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

		XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
		XMStoreFloat3(&n, unitNormal);

		return n;
	}

};

void MountObject::Build(const D3DApp* pApp)
{
	ID3D11Device* pDevice = pApp->GetDevice();

	GeometryGenerator::MeshData meshData;
	GeometryGenerator geo;
	geo.CreateGrid(LAND_SIZE,LAND_SIZE, 50,50, meshData);

	std::vector<Basic32> vertexs(meshData.Vertices.size());
	for( UINT i = 0; i < vertexs.size(); ++i )
	{
		auto const& meshVex = meshData.Vertices[i];
		vertexs[i].Pos = meshVex.Position;
		vertexs[i].Pos.y = MountHelper::GetHeight( vertexs[i].Pos.x, vertexs[i].Pos.z );
		vertexs[i].Normal = MountHelper::GetNormal( vertexs[i].Pos.x, vertexs[i].Pos.z );
		vertexs[i].Tex = meshVex.TexC;
	}

	mIndexNum = meshData.Indices.size();
	CreateBuffer( mVertexBuffer, pDevice, vertexs.size(), &vertexs[0], D3D11_BIND_VERTEX_BUFFER );
	CreateBuffer( mIndexBuffer, pDevice, mIndexNum, &meshData.Indices[0], D3D11_BIND_INDEX_BUFFER );

	HR(CreateDDSTextureFromFile(pDevice, L"Textures/grass.dds", nullptr, &mTextureSRV));

	mMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	XMMATRIX texScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mTexTransform, texScale);
}

void WaterObject::Build(const D3DApp* pApp)
{
	ID3D11Device* pDevice = pApp->GetDevice();

	const UINT WAVE_UNIT_NUM = 200;
	mWaves.Init( WAVE_UNIT_NUM, WAVE_UNIT_NUM, LAND_SIZE/WAVE_UNIT_NUM, 0.03f, 3.25f, 0.4f );

	CreateBuffer( mVertexBuffer, pDevice, mWaves.VertexCount(), (Basic32*)nullptr, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC );

	mIndexNum = 3*mWaves.TriangleCount();
	std::vector<UINT> indices(mIndexNum); // 3 indices per face
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for(UINT i = 0; i < m-1; ++i)
	{
		for(DWORD j = 0; j < n-1; ++j)
		{
			indices[k]   = i*n+j;
			indices[k+1] = i*n+j+1;
			indices[k+2] = (i+1)*n+j;

			indices[k+3] = (i+1)*n+j;
			indices[k+4] = i*n+j+1;
			indices[k+5] = (i+1)*n+j+1;

			k += 6; // next quad
		}
	}

	CreateBuffer( mIndexBuffer, pDevice, mIndexNum, &indices[0], D3D11_BIND_INDEX_BUFFER );

	HR(CreateDDSTextureFromFile(pDevice, L"Textures/water2.dds", nullptr, &mTextureSRV));

	mMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mTexTransform, I);

	mWaterTexOffset.x = mWaterTexOffset.y = 0;
	mNeedMap = true;
}

void WaterObject::Update(float dt)
{
	mTTotal+=dt;
	if( (mTTotal - mTBase) >= 0.1f )
	{
		mTBase += 0.1f;

		DWORD i = 5 + rand() % (mWaves.RowCount()-10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount()-10);

		float r = MathHelper::RandF(0.5f, 1.0f);

		mWaves.Disturb(i, j, r);
		mNeedMap = true;
	}

	if( mWaves.Update(dt) )
	{
		mNeedMap = true;
	}

	//
	// Animate water texture coordinates.
	//

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	mWaterTexOffset.y += 0.05f*dt;
	mWaterTexOffset.x += 0.1f*dt;	
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&mTexTransform, wavesScale*wavesOffset);
}

void WaterObject::DrawTransparency(const D3DApp* pApp) 
{
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	pD3dContext->OMSetDepthStencilState(DSSNoDepth::Ptr(), 1);
	pD3dContext->OMSetBlendState( BSTransparent::Ptr() , blendFactor, 0xffffffff );

	if( mNeedMap )
	{
		// Update the wave vertex buffer with the new solution.
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(pD3dContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

		Basic32* v = reinterpret_cast<Basic32*>(mappedData.pData);
		for(UINT i = 0; i < mWaves.VertexCount(); ++i)
		{
			v[i].Pos    = mWaves[i];
			v[i].Normal = mWaves.Normal(i);

			// Derive Tex-Coords in [0,1] from position.
			v[i].Tex.x  = 0.5f + mWaves[i].x / mWaves.Width();
			v[i].Tex.y  = 0.5f - mWaves[i].z / mWaves.Depth();
		}

		pD3dContext->Unmap(mVertexBuffer, 0);
		mNeedMap = false;
	}

	RenderObject::Draw(pApp);

	pD3dContext->OMSetBlendState( nullptr , blendFactor, 0xffffffff );
	pD3dContext->OMSetDepthStencilState(nullptr, 0);
}

void TreeObject::Build(const D3DApp* pApp)
{
	ID3D11Device* pDevice = pApp->GetDevice();
	ID3D11DeviceContext* pContext = pApp->GetContext();

	mMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	std::vector<std::wstring> treeFilenames;
	treeFilenames.push_back(L"Textures/tree0.dds");
	treeFilenames.push_back(L"Textures/tree1.dds");
	treeFilenames.push_back(L"Textures/tree2.dds");
	treeFilenames.push_back(L"Textures/tree3.dds");
	mTextureSRV = d3dHelper::CreateTexture2DArraySRV(pDevice,pContext,treeFilenames);


	const UINT TreeCount = 16;
	TreePointSprite v[TreeCount];

	for(UINT i = 0; i < TreeCount; ++i)
	{
		float x(0),y(0),z(0);
		const float TREE_LAND_HEIGHT = 5.f;
		const float TREE_AREA = LAND_SIZE/2.f - 10.f;

		while ( y < TREE_LAND_HEIGHT )
		{
			x = MathHelper::RandF(-TREE_AREA, TREE_AREA);
			z = MathHelper::RandF(-TREE_AREA, TREE_AREA);
			y = MountHelper::GetHeight(x,z);
		}

		// Move tree slightly above land height.
		y += 12.0f;

		v[i].Pos  = XMFLOAT3(x,y,z);
		v[i].Size = XMFLOAT2(24.0f, 24.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(TreePointSprite) * TreeCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(pDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer));
}

void TreeObject::Draw(const D3DApp* pApp) 
{
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();
	ID3DX11EffectTechnique* pTech = TreeSpriteEffect::Ptr()->GetTechnique(pApp->GetRenderOptions());

	TreeSpriteEffect::Ptr()->SetMaterial(mMaterial);
	TreeSpriteEffect::Ptr()->SetTreeTextureMapArray(mTextureSRV);
	TreeSpriteEffect::Ptr()->SetEyePosW(pApp->GetCamera().GetPosition());
	TreeSpriteEffect::Ptr()->SetViewProj(pApp->GetCamera().ViewProj());

	pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	pD3dContext->IASetInputLayout(TreePointSprite::GetInputLayout());
	UINT stride = sizeof(TreePointSprite);
	UINT offset = 0;

	D3DX11_TECHNIQUE_DESC techDesc;
	pTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		pD3dContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		pTech->GetPassByIndex(p)->Apply(0, pD3dContext);
		pD3dContext->Draw(16, 0);
	}
}

const float WALL_GROUND_HEIGHT = 3.f;

void SkullObject::Build(const D3DApp* pApp)
{
	BasicSkullObject::Build( pApp );

	mShadowMat.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mShadowMat.Diffuse  = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	mShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	mSkullTranslation = XMFLOAT3(0.0f, WALL_GROUND_HEIGHT+1.0f, -5.0f);
}

void SkullObject::Draw(const D3DApp* pApp) 
{
	ID3DX11EffectTechnique* pTech = pApp->GetRenderOptions()==TexturesAndFog? BasicEffect::Ptr()->Light3FogTech:BasicEffect::Ptr()->Light3Tech;
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();
	SetupEffect( BasicEffect::Ptr(), pApp );
	DrawWithTech(pD3dContext,pTech);

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	UINT stride = sizeof(Basic32);
	UINT offset = 0;
	//
	// Draw the mirror to stencil buffer only.
	//
	D3DX11_TECHNIQUE_DESC techDesc;
	pTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = pTech->GetPassByIndex( p );

		pD3dContext->IASetVertexBuffers(0, 1, mWall->GetVertexBuffer(), &stride, &offset);

		// Set per object constants.
		mWall->SetupEffect( BasicEffect::Ptr(), pApp );

		//
		//pD3dContext->RSSetState(RenderStates::NoCullRS);

		// Do not write to render target.
		pD3dContext->OMSetBlendState(BSNoRenderTargetWrites::Ptr(), blendFactor, 0xffffffff);

		// Render visible mirror pixels to stencil buffer.
		// Do not write mirror depth to depth buffer at this point, otherwise it will occlude the reflection.
		pD3dContext->OMSetDepthStencilState(DDSMarkMirror::Ptr(), 1);

		pass->Apply(0, pD3dContext);
		pD3dContext->Draw(6, 24);

		// Restore states.
		pD3dContext->OMSetDepthStencilState(0, 0);
		pD3dContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		//pD3dContext->RSSetState(nullptr);
	}


	//
	// Draw the skull reflection.
	//
	pTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = pTech->GetPassByIndex( p );

		pD3dContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		pD3dContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
		XMMATRIX R = XMMatrixReflect(mirrorPlane);
		XMMATRIX world = XMLoadFloat4x4(&mWorld) * R;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*pApp->GetCamera().ViewProj();

		BasicEffect::Ptr()->SetWorld(world);
		BasicEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
		BasicEffect::Ptr()->SetWorldViewProj(worldViewProj);
		BasicEffect::Ptr()->SetMaterial(mMaterial);

		// Cache the old light directions, and reflect the light directions.
		XMFLOAT3 oldLightDirections[3];
		XMFLOAT3 reflectLight[3];
		DirectionalLight* pDirLights = const_cast<DirectionalLight*>(static_cast<const D11DemoApp*>(pApp)->GetLights());
		for(int i = 0; i < 3; ++i)
		{
			oldLightDirections[i] = pDirLights[i].Direction;

			XMVECTOR lightDir = XMLoadFloat3(&pDirLights[i].Direction);
			XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
			XMStoreFloat3(&pDirLights[i].Direction, reflectedLightDir);
		}

		BasicEffect::Ptr()->SetDirLights(pDirLights);

		// Cull clockwise triangles for reflection.
		pD3dContext->RSSetState(RSCullClockwise::Ptr());

		// Only draw reflection into visible mirror pixels as marked by the stencil buffer. 
		pD3dContext->OMSetDepthStencilState(DSSDrawReflection::Ptr(), 1);
		pass->Apply(0, pD3dContext);
		pD3dContext->DrawIndexed(mIndexNum, 0, 0);

		// Restore default states.
		pD3dContext->RSSetState(0);	
		pD3dContext->OMSetDepthStencilState(0, 0);	

		// Restore light directions.
		for(int i = 0; i < 3; ++i)
		{
			pDirLights[i].Direction = oldLightDirections[i];
		}

		BasicEffect::Ptr()->SetDirLights(pDirLights);
	}
}

void SkullObject::DrawTransparency(const D3DApp* pApp) 
{
	ID3DX11EffectTechnique* pTech = pApp->GetRenderOptions()==TexturesAndFog? BasicEffect::Ptr()->Light3FogTech:BasicEffect::Ptr()->Light3Tech;
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();

	pD3dContext->IASetInputLayout(Basic32::GetInputLayout());
	pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Basic32);
	UINT offset = 0;
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	//
	// Draw the ground to stencil buffer only.
	//
	D3DX11_TECHNIQUE_DESC techDesc;
	pTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = pTech->GetPassByIndex( p );

		pD3dContext->IASetVertexBuffers(0, 1, mWall->GetVertexBuffer(), &stride, &offset);

		// Set per object constants.
		mWall->SetupEffect( BasicEffect::Ptr(), pApp );
		//
		//pD3dContext->RSSetState(RenderStates::NoCullRS);

		// Do not write to render target.
		pD3dContext->OMSetBlendState(BSNoRenderTargetWrites::Ptr(), blendFactor, 0xffffffff);

		// Render visible mirror pixels to stencil buffer.
		// Do not write mirror depth to depth buffer at this point, otherwise it will occlude the reflection.
		pD3dContext->OMSetDepthStencilState(DDSMarkMirror::Ptr(), 1<<1);

		pass->Apply(0, pD3dContext);
		pD3dContext->Draw(6, 0);

		// Restore states.
		pD3dContext->OMSetDepthStencilState(0, 0);
		pD3dContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		//pD3dContext->RSSetState(nullptr);
	}


	pD3dContext->IASetVertexBuffers( 0, 1, &mVertexBuffer,&stride,&offset );
	pD3dContext->IASetIndexBuffer(mIndexBuffer,DXGI_FORMAT_R32_UINT,0);
	pTech->GetDesc( &techDesc );
	const D11DemoApp* demoApp = static_cast<const D11DemoApp*>(pApp);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = pTech->GetPassByIndex( p );

		XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
		XMVECTOR toMainLight = -XMLoadFloat3( &( demoApp->GetLights()->Direction ) );
		XMMATRIX S =  XMMatrixShadow(shadowPlane, toMainLight);
		XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, WALL_GROUND_HEIGHT+0.001f, 0.0f);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mWorld)*S*shadowOffsetY;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*pApp->GetCamera().ViewProj();

		BasicEffect::Ptr()->SetWorld(world);
		BasicEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
		BasicEffect::Ptr()->SetWorldViewProj(worldViewProj);
		BasicEffect::Ptr()->SetMaterial(mShadowMat);
		BasicEffect::Ptr()->SetTexTransform( XMLoadFloat4x4( &mTexTransform ) );

		pD3dContext->OMSetBlendState(BSTransparent::Ptr(), blendFactor, 0xffffffff);
		pD3dContext->OMSetDepthStencilState(DSSStencilNoDoubleBlend::Ptr(), 1<<1);
		pass->Apply(0, pD3dContext);
		pD3dContext->DrawIndexed(mIndexNum, 0, 0);

		// Restore default states.
		pD3dContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		pD3dContext->OMSetDepthStencilState(0, 0);
	}
}

void SkullObject::Update(float dt)
{
	//
	// Allow user to move box.
	//
	if( GetAsyncKeyState('A') & 0x8000 )
		mSkullTranslation.x -= 1.0f*dt;

	if( GetAsyncKeyState('D') & 0x8000 )
		mSkullTranslation.x += 1.0f*dt;

	if( GetAsyncKeyState('W') & 0x8000 )
		mSkullTranslation.y += 1.0f*dt;

	if( GetAsyncKeyState('S') & 0x8000 )
		mSkullTranslation.y -= 1.0f*dt;

	// Don't let user move below ground plane.
	mSkullTranslation.y = MathHelper::Max(mSkullTranslation.y, WALL_GROUND_HEIGHT);

	// Update the new world matrix.
	XMMATRIX skullRotate = XMMatrixRotationY(0.5f*MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
	XMStoreFloat4x4(&mWorld, skullRotate*skullScale*skullOffset);
}

WallObject::~WallObject()
{
	ReleaseCOM(mFloorDiffuseMapSRV);
	ReleaseCOM(mWallDiffuseMapSRV);
	ReleaseCOM(mMirrorDiffuseMapSRV);
}

void WallObject::Build(const D3DApp* pApp)
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/


	const UINT VERTEX_NUM = 30;

	Basic32 v[VERTEX_NUM];

	// Floor: Observe we tile texture coordinates.
	v[0] = Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[1] = Basic32(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Basic32( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

	v[3] = Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[4] = Basic32( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	v[5] = Basic32( 7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

	// Wall: Observe we tile texture coordinates, and that we
	// leave a gap in the middle for the mirror.
	v[6]  = Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[7]  = Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8]  = Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);

	v[9]  = Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[10] = Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
	v[11] = Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

	v[12] = Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[13] = Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

	v[15] = Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[16] = Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[17] = Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	v[18] = Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Basic32( 7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);

	v[21] = Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Basic32( 7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
	v[23] = Basic32( 7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);

	// Mirror
	v[24] = Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[25] = Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[26] = Basic32( 2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[27] = Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[28] = Basic32( 2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[29] = Basic32( 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	ID3D11Device* pDevice = pApp->GetDevice();

	CreateBuffer(mVertexBuffer,pDevice,VERTEX_NUM,v,D3D11_BIND_VERTEX_BUFFER);


	HR(CreateDDSTextureFromFile(pDevice, L"Textures/checkboard.dds", nullptr, &mFloorDiffuseMapSRV));
	HR(CreateDDSTextureFromFile(pDevice, L"Textures/brick01.dds", nullptr, &mWallDiffuseMapSRV));
	HR(CreateDDSTextureFromFile(pDevice, L"Textures/ice.dds", nullptr, &mMirrorDiffuseMapSRV));

	XMMATRIX mat = XMMatrixTranslation(0, WALL_GROUND_HEIGHT, 0.0f);
	XMStoreFloat4x4(&mWorld, mat);

	mMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mMirrorMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMirrorMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mMirrorMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}

void WallObject::Draw(const D3DApp* pApp) 
{
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();
	ID3DX11EffectTechnique* pTech = BasicEffect::Ptr()->GetTechnique(pApp->GetRenderOptions());

	SetupEffect( BasicEffect::Ptr(), pApp );

	pD3dContext->IASetInputLayout(Basic32::GetInputLayout());
	pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pD3dContext->RSSetState(RSNoCull::Ptr());

	UINT stride = sizeof(Basic32);
	UINT offset = 0;
	pD3dContext->IASetVertexBuffers( 0, 1, &mVertexBuffer,&stride,&offset );

	D3DX11_TECHNIQUE_DESC techDesc;
	pTech->GetDesc(&techDesc);

	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = pTech->GetPassByIndex( p );

		BasicEffect::Ptr()->SetMaterial( mMaterial );
		// Set per object constants.
		BasicEffect::Ptr()->SetDiffuseMap(mFloorDiffuseMapSRV);
		pass->Apply(0, pD3dContext);
		pD3dContext->Draw(6, 0);

		// Wall
		BasicEffect::Ptr()->SetDiffuseMap(mWallDiffuseMapSRV);
		pass->Apply(0, pD3dContext);
		pD3dContext->Draw(18, 6);
	}

	pD3dContext->RSSetState(nullptr);
}

void WallObject::DrawTransparency(const D3DApp* pApp) 
{
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();
	ID3DX11EffectTechnique* pTech = BasicEffect::Ptr()->GetTechnique(pApp->GetRenderOptions());

	SetupEffect( BasicEffect::Ptr(), pApp );

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	pD3dContext->IASetInputLayout(Basic32::GetInputLayout());
	pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pD3dContext->RSSetState(RSNoCull::Ptr());

	UINT stride = sizeof(Basic32);
	UINT offset = 0;
	pD3dContext->IASetVertexBuffers( 0, 1, &mVertexBuffer,&stride,&offset );

	D3DX11_TECHNIQUE_DESC techDesc;
	pTech->GetDesc(&techDesc);

	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = pTech->GetPassByIndex( p );
		// Mirror
		BasicEffect::Ptr()->SetMaterial( mMirrorMaterial );
		pD3dContext->OMSetDepthStencilState(DSSNoDepth::Ptr(), 1);
		pD3dContext->OMSetBlendState(BSTransparent::Ptr(), blendFactor, 0xffffffff);
		BasicEffect::Ptr()->SetDiffuseMap(mMirrorDiffuseMapSRV);
		pass->Apply(0, pD3dContext);
		pD3dContext->Draw(6, 24);
		pD3dContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
		pD3dContext->OMSetDepthStencilState(nullptr, 0);
	}

	pD3dContext->RSSetState(nullptr);
}

void FullScreenQuadObject::Build(const D3DApp* pApp)
{
	ID3D11Device* pDevice = pApp->GetDevice();

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
	mIndexNum = quad.Indices.size();
	CreateBuffer( mVertexBuffer, pDevice, quad.Vertices.size(), &vertices[0], D3D11_BIND_VERTEX_BUFFER );
	CreateBuffer( mIndexBuffer, pDevice, mIndexNum, &quad.Indices[0], D3D11_BIND_INDEX_BUFFER );
}

void FullScreenQuadObject::DrawQuad(const D3DApp* pApp,ID3D11ShaderResourceView* tex) const
{
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();
	BasicEffect* pBasicFx = BasicEffect::Ptr();
	ID3DX11EffectTechnique* texOnlyTech = pBasicFx->Light0TexTech;
	XMMATRIX identity = XMMatrixIdentity();

	D3DX11_TECHNIQUE_DESC techDesc;
	UINT stride = sizeof(Basic32);
	UINT offset = 0;

	texOnlyTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		pD3dContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		pD3dContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		pBasicFx->SetWorld(identity);
		pBasicFx->SetWorldInvTranspose(identity);
		pBasicFx->SetWorldViewProj(identity);
		pBasicFx->SetTexTransform(identity);
		pBasicFx->SetDiffuseMap(tex);

		texOnlyTech->GetPassByIndex(p)->Apply(0, pD3dContext);
		pD3dContext->DrawIndexed(mIndexNum, 0, 0);
	}
}

CsWaterObject::CsWaterObject()
	:mWaterOutputUAV(nullptr)
	,mWaterOutputBuffer(nullptr)
	,mWaveHeightUAV(0)
	,mDisturbCounter(0)
	,mWaveCounter(0.03f)
{

}

CsWaterObject::~CsWaterObject()
{
	ReleaseCOM(mWaterOutputUAV);
	ReleaseCOM(mWaterOutputBuffer);
	ReleaseCOM(mWaveHeightUAV);
}

void CsWaterObject::Build(const D3DApp* pApp)
{
	ID3D11Device* pDevice = pApp->GetDevice();

	GeometryGenerator::MeshData meshData;
	GeometryGenerator geo;
	geo.CreateGrid( LAND_SIZE,LAND_SIZE, VERTEX_NUM, VERTEX_NUM, meshData );

	std::vector<Basic32> vertexs(meshData.Vertices.size());
	for( UINT i = 0; i < vertexs.size(); ++i )
	{
		auto const& meshVex = meshData.Vertices[i];
		vertexs[i].Pos = meshVex.Position;
		vertexs[i].Pos.y = 0;
		vertexs[i].Normal = XMFLOAT3(0,1,0);
		vertexs[i].Tex = meshVex.TexC;
	}

	mIndexNum = meshData.Indices.size();
	CreateBuffer( mVertexBuffer, pDevice, vertexs.size(), &vertexs[0], D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT );
	CreateBuffer( mIndexBuffer, pDevice, mIndexNum, &meshData.Indices[0], D3D11_BIND_INDEX_BUFFER );

	//Create compute shader output buffer
	int vertexBufferNum = vertexs.size();
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	vbd.ByteWidth = sizeof(Basic32)*vertexBufferNum;
	vbd.StructureByteStride = sizeof(Basic32);
	vbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &vertexs[0];
	HR( pDevice->CreateBuffer( &vbd, &initData, &mWaterOutputBuffer ) );

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = vertexBufferNum;
	HR(pDevice->CreateUnorderedAccessView(mWaterOutputBuffer, &uavDesc, &mWaterOutputUAV));

	//Create compute cache height buffer
	ID3D11Buffer*	pWaveHeightBuffer;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	vbd.ByteWidth = sizeof(float)*vertexBufferNum;
	vbd.StructureByteStride = sizeof(float);
	vbd.MiscFlags = vbd.CPUAccessFlags = 0;

	std::vector<float> heights(vertexBufferNum);
	initData.pSysMem = &heights[0];
	HR( pDevice->CreateBuffer( &vbd, &initData, &pWaveHeightBuffer ) );

	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = vertexBufferNum;
	HR(pDevice->CreateUnorderedAccessView(pWaveHeightBuffer, &uavDesc, &mWaveHeightUAV));
	ReleaseCOM(pWaveHeightBuffer);

	HR(CreateDDSTextureFromFile(pDevice, L"Textures/water2.dds", nullptr, &mTextureSRV));

	mMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mTexTransform, I);
	mWaterTexOffset.x = mWaterTexOffset.y = 0;
}

void CsWaterObject::DrawTransparency(const D3DApp* pApp) 
{
	ID3DX11EffectTechnique* pTech = WaterEffect::Ptr()->GetTechnique(pApp->GetRenderOptions());
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();

	SetupEffect( WaterEffect::Ptr(), pApp );

	bool needCopy = false;

	if( mDisturbCounter <= 0 )
	{
		needCopy = true;
		DrawDisturb(pD3dContext);
	}

	if( mWaveCounter <= 0 )
	{
		needCopy = true;
		DrawUpdateWave(pD3dContext);
	}

	if( needCopy )
	{
		pD3dContext->CopyResource(mVertexBuffer,mWaterOutputBuffer);
	}

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	pD3dContext->OMSetDepthStencilState(DSSNoDepth::Ptr(), 1);
	pD3dContext->OMSetBlendState( BSTransparent::Ptr() , blendFactor, 0xffffffff );

	UINT stride = sizeof(Basic32);
	UINT offset = 0;
	pD3dContext->IASetInputLayout(Basic32::GetInputLayout());
	pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pD3dContext->IASetVertexBuffers( 0, 1, &mVertexBuffer,&stride,&offset );
	pD3dContext->IASetIndexBuffer(mIndexBuffer,DXGI_FORMAT_R32_UINT,0);

	D3DX11_TECHNIQUE_DESC techDesc;
	pTech->GetDesc(&techDesc);

	for(UINT p=0;p<techDesc.Passes;++p)
	{
		pTech->GetPassByIndex(p)->Apply(0,pD3dContext);
		pD3dContext->DrawIndexed(mIndexNum,0,0);
	}

	pD3dContext->OMSetBlendState( nullptr , blendFactor, 0xffffffff );
	pD3dContext->OMSetDepthStencilState(nullptr, 0);
}

void CsWaterObject::Update(float dt)
{
	if( mDisturbCounter <= 0 )
	{
		mDisturbCounter = 0.1f;
	}
	mDisturbCounter -= dt;

	if( mWaveCounter <= 0 )
	{
		mWaveCounter = 0.03f;
	}
	mWaveCounter -= dt;

	// Translate texture over time.
	mWaterTexOffset.y += 0.05f*dt;
	mWaterTexOffset.x += 0.1f*dt;	
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&mTexTransform, wavesScale*wavesOffset);
}

void CsWaterObject::DrawUpdateWave(ID3D11DeviceContext* pD3dContext) const
{
	D3DX11_TECHNIQUE_DESC techDesc;
	WaterEffect::Ptr()->WaveMotionTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		WaterEffect::Ptr()->SetWaveSize(XMINT2(VERTEX_NUM,VERTEX_NUM));
		WaterEffect::Ptr()->SetWaveOutput(mWaterOutputUAV);
		WaterEffect::Ptr()->SetWavePreY(mWaveHeightUAV);

		WaterEffect::Ptr()->WaveMotionTech->GetPassByIndex(p)->Apply(0, pD3dContext);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf((VERTEX_NUM-2)*(VERTEX_NUM-2) / 256.0f);
		pD3dContext->Dispatch(numGroupsX, 1, 1);
	}

	// Unbind output from compute shader (we are going to use this output as an input in the next pass, 
	// and a resource cannot be both an output and input at the same time.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	pD3dContext->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
}

void CsWaterObject::DrawDisturb(ID3D11DeviceContext* pD3dContext) const
{
	D3DX11_TECHNIQUE_DESC techDesc;
	WaterEffect::Ptr()->WaveDisturbTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		int i = 5 + rand() % (VERTEX_NUM-10);
		int j = 5 + rand() % (VERTEX_NUM-10);
		float r = MathHelper::RandF(0.5f, 1.0f);

		WaterEffect::Ptr()->SetDisturbIndex(XMINT2(i,j));
		WaterEffect::Ptr()->SetDisturbHeight(r);
		WaterEffect::Ptr()->WaveDisturbTech->GetPassByIndex(p)->Apply(0, pD3dContext);

		pD3dContext->Dispatch(1, 1, 1);
	}
}