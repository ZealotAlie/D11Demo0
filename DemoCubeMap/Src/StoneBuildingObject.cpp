#include "StoneBuildingObject.h"
#include "d3dApp.h"
#include "UnknownInstance\Basic32.h"
#include "GeometryGenerator.h"

void StoneBuildingObject::Build(const D3DApp* pApp)
{
	ID3D11Device* pDevice = pApp->GetDevice();

	HR(CreateDDSTextureFromFile(pDevice, L"Textures/floor.dds", nullptr, &mFloorTexSRV));
	HR(CreateDDSTextureFromFile(pDevice, L"Textures/stone.dds", nullptr, &mStoneTexSRV));
	HR(CreateDDSTextureFromFile(pDevice, L"Textures/bricks.dds", nullptr, &mBrickTexSRV));


	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset      = 0;
	mGridVertexOffset     = box.Vertices.size();
	mSphereVertexOffset   = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	mBoxIndexCount      = box.Indices.size();
	mGridIndexCount     = grid.Indices.size();
	mSphereIndexCount   = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset      = 0;
	mGridIndexOffset     = mBoxIndexCount;
	mSphereIndexOffset   = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount = 
		box.Vertices.size() + 
		grid.Vertices.size() + 
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount = 
		mBoxIndexCount + 
		mGridIndexCount + 
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex    = box.Vertices[i].TexC;
	}

	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex    = grid.Vertices[i].TexC;
	}

	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex    = sphere.Vertices[i].TexC;
	}

	for(size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex    = cylinder.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(pDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer));

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
	HR(pDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer));
}

void StoneBuildingObject::Draw(const D3DApp* pApp)
{
	ID3D11DeviceContext* pContext = pApp->GetContext();

	XMMATRIX view     = pApp->GetCamera().View();
	XMMATRIX proj     = pApp->GetCamera().Proj();
	XMMATRIX viewProj = pApp->GetCamera().ViewProj();

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	// Set per frame constants.
	BasicEffect::Ptr()->SetEyePosW(pApp->GetCamera().GetPosition());
	//BasicEffect::Ptr()->SetCubeMap(mSky->CubeMapSRV());

	// Figure out which technique to use.  Skull does not have texture coordinates,
	// so we need a separate technique for it, and not every surface is reflective,
	// so don't pay for cubemap look up.

	ID3DX11EffectTechnique* activeTexTech     = BasicEffect::Ptr()->Light1TexTech;
	ID3DX11EffectTechnique* activeReflectTech = BasicEffect::Ptr()->Light1TexReflectTech;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	pContext->IASetInputLayout(Basic32::GetInputLayout());
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Basic32);
	UINT offset = 0;
	//
	// Draw the grid, cylinders, and box without any cubemap reflection.
	// 
	D3DX11_TECHNIQUE_DESC techDesc;
	activeTexTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		pContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Draw the grid.
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		BasicEffect::Ptr()->SetWorld(world);
		BasicEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
		BasicEffect::Ptr()->SetWorldViewProj(worldViewProj);
		BasicEffect::Ptr()->SetTexTransform(XMMatrixScaling(6.0f, 8.0f, 1.0f));
		BasicEffect::Ptr()->SetMaterial(mGridMat);
		BasicEffect::Ptr()->SetDiffuseMap(mFloorTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, pContext);
		pContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		BasicEffect::Ptr()->SetWorld(world);
		BasicEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
		BasicEffect::Ptr()->SetWorldViewProj(worldViewProj);
		BasicEffect::Ptr()->SetTexTransform(XMMatrixIdentity());
		BasicEffect::Ptr()->SetMaterial(mBoxMat);
		BasicEffect::Ptr()->SetDiffuseMap(mStoneTexSRV);

		activeTexTech->GetPassByIndex(p)->Apply(0, pContext);
		pContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw the cylinders.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			BasicEffect::Ptr()->SetWorld(world);
			BasicEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
			BasicEffect::Ptr()->SetWorldViewProj(worldViewProj);
			BasicEffect::Ptr()->SetTexTransform(XMMatrixIdentity());
			BasicEffect::Ptr()->SetMaterial(mCylinderMat);
			BasicEffect::Ptr()->SetDiffuseMap(mBrickTexSRV);

			activeTexTech->GetPassByIndex(p)->Apply(0, pContext);
			pContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}
	}

	//
	// Draw the spheres with cubemap reflection.
	//
	activeReflectTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the spheres.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			BasicEffect::Ptr()->SetWorld(world);
			BasicEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
			BasicEffect::Ptr()->SetWorldViewProj(worldViewProj);
			BasicEffect::Ptr()->SetTexTransform(XMMatrixIdentity());
			BasicEffect::Ptr()->SetMaterial(mSphereMat);
			BasicEffect::Ptr()->SetDiffuseMap(mStoneTexSRV);

			activeReflectTech->GetPassByIndex(p)->Apply(0, pContext);
			pContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}
}

void ReflectSkullObject::Build(const D3DApp* pApp)
{
	BasicSkullObject::Build(pApp);

	mMaterial.Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mMaterial.Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mMaterial.Reflect  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mWorld, XMMatrixMultiply(skullScale, skullOffset));
}

void ReflectSkullObject::Draw(const D3DApp* pApp)
{
	ID3D11DeviceContext* pContext = pApp->GetContext();

	pContext->IASetInputLayout(Basic32::GetInputLayout());
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	ID3DX11EffectTechnique* activeSkullTech   = BasicEffect::Ptr()->Light3ReflectTech;

	UINT stride = sizeof(Basic32);
	UINT offset = 0;

	D3DX11_TECHNIQUE_DESC techDesc;

	activeSkullTech->GetDesc( &techDesc );

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the skull.

		pContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&mWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*pApp->GetCamera().ViewProj();

		BasicEffect::Ptr()->SetWorld(world);
		BasicEffect::Ptr()->SetWorldInvTranspose(worldInvTranspose);
		BasicEffect::Ptr()->SetWorldViewProj(worldViewProj);
		BasicEffect::Ptr()->SetMaterial(mMaterial);

		activeSkullTech->GetPassByIndex(p)->Apply(0, pContext);
		pContext->DrawIndexed(mIndexNum, 0, 0);
	}
}