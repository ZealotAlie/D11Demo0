#include "CommonRenderObjects.h"
#include "GeometryGenerator.h"
#include "UnknownInstance\Basic32.h"
#include "UnknownInstance\Pos.h"

void BasicSkullObject::Build(const D3DApp* pApp)
{
	std::ifstream fin("Models/skull.txt");

	if(!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	std::vector<Basic32> vertices(vcount);
	for(UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mSkullBox.Center, 0.5f*(vMin+vMax));
	XMStoreFloat3(&mSkullBox.Extents, 0.5f*(vMax-vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mIndexNum = 3*tcount;
	std::vector<UINT> indices(mIndexNum);
	for(UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
	}

	fin.close();

	mMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	XMMATRIX skullMat = XMMatrixTranslation(0, 20, 0.0f);
	XMStoreFloat4x4(&mWorld, skullMat);

	ID3D11Device* pDevice = pApp->GetDevice();
	CreateBuffer( mVertexBuffer, pDevice, vcount, &vertices[0], D3D11_BIND_VERTEX_BUFFER );
	CreateBuffer( mIndexBuffer, pDevice, mIndexNum, &indices[0], D3D11_BIND_INDEX_BUFFER );
}


SkyObject::~SkyObject()
{
	for(auto pTex : mSkyTextures)
	{
		ReleaseCOM(pTex);
	}
	mSkyTextures.clear();
	mTextureSRV = nullptr;
}

void SkyObject::Build(const D3DApp* pApp,const std::vector<std::wstring> cubemapFilenames, float skySphereRadius)
{
	ID3D11Device* pDevice = pApp->GetDevice();


	ID3D11ShaderResourceView*	pOutput(nullptr);
	for(auto const& file :cubemapFilenames)
	{
		HR(CreateDDSTextureFromFile(pDevice, file.c_str(), nullptr, &pOutput));
		mSkyTextures.push_back(pOutput);
	}
	mTextureSRV = mSkyTextures[0];

	GeometryGenerator::MeshData sphere;
	GeometryGenerator geoGen;
	geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

	std::vector<XMFLOAT3> vertices( sphere.Vertices.size() );

	for(size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		vertices[i] = sphere.Vertices[i].Position;
	}

	mIndexNum = sphere.Indices.size();
	CreateBuffer( mVertexBuffer, pDevice, vertices.size(), &vertices[0], D3D11_BIND_VERTEX_BUFFER );

	std::vector<USHORT> indices16;
	indices16.assign(sphere.Indices.begin(), sphere.Indices.end());
	CreateBuffer( mIndexBuffer, pDevice, mIndexNum, &indices16[0], D3D11_BIND_INDEX_BUFFER );
}

void SkyObject::Draw(const D3DApp* pApp)
{
	ID3D11DeviceContext* pContext = pApp->GetContext();
	// center Sky about eye in world space
	XMFLOAT3 eyePos = pApp->GetCamera().GetPosition();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);


	XMMATRIX WVP = XMMatrixMultiply(T, pApp->GetCamera().ViewProj());

	SkyEffect::Ptr()->SetWorldViewProj(WVP);
	SkyEffect::Ptr()->SetCubeMap(mTextureSRV);


	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	pContext->IASetInputLayout(Pos::GetInputLayout());
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DX11_TECHNIQUE_DESC techDesc;
	SkyEffect::Ptr()->SkyTech->GetDesc( &techDesc );

	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = SkyEffect::Ptr()->SkyTech->GetPassByIndex(p);

		pass->Apply(0, pContext);

		pContext->DrawIndexed(mIndexNum, 0, 0);
	}
	// restore default states, as the SkyFX changes them in the effect file.
	pContext->RSSetState(0);
	pContext->OMSetDepthStencilState(0, 0);
}

StoneBuildingBase::StoneBuildingBase()
	:mFloorTexSRV(nullptr)
	,mStoneTexSRV(nullptr)
	,mBrickTexSRV(nullptr)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	for(int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i*2+0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mCylWorld[i*2+1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		XMStoreFloat4x4(&mSphereWorld[i*2+0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mSphereWorld[i*2+1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}

	mGridMat.Ambient  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mGridMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mCylinderMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient  = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Diffuse  = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	mBoxMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

StoneBuildingBase::~StoneBuildingBase()
{
	ReleaseCOM(mFloorTexSRV);
	ReleaseCOM(mStoneTexSRV);
	ReleaseCOM(mBrickTexSRV);
}