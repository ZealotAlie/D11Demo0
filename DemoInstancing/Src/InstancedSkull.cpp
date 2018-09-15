#include "InstancedSkull.h"
#include "UnknownInstance\Basic32.h"
#include "InstancingEffect.h"
#include <DirectXCollision.h>

void InstancedSkull::Update(float dt)
{
	D3DApp* pApp = D3DApp::GetInstance();
	ID3D11DeviceContext* pContext = pApp->GetContext();

	mVisibleObjectCount = 0;

	D3D11_MAPPED_SUBRESOURCE mappedData; 
	pContext->Map(mInstanceVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

	switch( mCullingType )
	{
	case CullingNone:
		{
			for(UINT i = 0; i < mInstancedData.size(); ++i)
			{
				dataView[mVisibleObjectCount++] = mInstancedData[i];
			}
		}
		break;

	case LocalFrustumCulling:
		{
			XMVECTOR detView = XMMatrixDeterminant(pApp->GetCamera().View());
			XMMATRIX invView = XMMatrixInverse(&detView, pApp->GetCamera().View());

			for(UINT i = 0; i < mInstancedData.size(); ++i)
			{
				XMMATRIX W = XMLoadFloat4x4(&mInstancedData[i].World);
				XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

				// View space to the object's local space.
				XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

				// Decompose the matrix into its individual parts.
				XMVECTOR scale;
				XMVECTOR rotQuat;
				XMVECTOR translation;
				XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

				// Transform the camera frustum from view space to the object's local space.
				BoundingFrustum localspaceFrustum;
				pApp->GetCameraFrustum().Transform( localspaceFrustum, XMVectorGetX(scale), rotQuat, translation );

				// Perform the box/frustum intersection test in local space.
				if(localspaceFrustum.Intersects(mSkullBox) != 0)
				{
					// Write the instance data to dynamic VB of the visible objects.
					dataView[mVisibleObjectCount++] = mInstancedData[i];
				}
			}
		}
		break;
	case CameraFrustumCulling:
		{
			XMMATRIX V = pApp->GetCamera().View();
			BoundingBox	frustumSkullBox;

			for(UINT i = 0; i < mInstancedData.size(); ++i)
			{
				XMMATRIX W = XMLoadFloat4x4(&mInstancedData[i].World);
				XMMATRIX worldView = XMMatrixMultiply(W, V);
				mSkullBox.Transform( frustumSkullBox, worldView );
				if(pApp->GetCameraFrustum().Intersects(frustumSkullBox) != 0)
				{
					// Write the instance data to dynamic VB of the visible objects.
					dataView[mVisibleObjectCount++] = mInstancedData[i];
				}
			}
		}
		break;
	}

	pContext->Unmap(mInstanceVB, 0);
}

void InstancedSkull::Build(const D3DApp* pApp)
{
	BasicSkullObject::Build(pApp);
	BuildInstanceBuffer(pApp->GetDevice());
}

void InstancedSkull::Draw(const D3DApp* pApp)
{
	ID3D11DeviceContext* pContext = pApp->GetContext();

	pContext->IASetInputLayout(InstancedData::GetInputLayout());
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	InstancingEffect::Ptr()->SetEyePosW(pApp->GetCamera().GetPosition());
	InstancingEffect::Ptr()->SetViewProj(pApp->GetCamera().ViewProj());
	InstancingEffect::Ptr()->SetMaterial(mMaterial);

	UINT stride[2] = {sizeof(Basic32), sizeof(InstancedData)};
	UINT offset[2] = {0,0};

	ID3D11Buffer* vbs[2] = {mVertexBuffer, mInstanceVB};

	ID3DX11EffectTechnique* activeTech = InstancingEffect::Ptr()->Light3Tech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the skull.

		pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		pContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		InstancingEffect::Ptr()->SetMaterial(mMaterial);

		activeTech->GetPassByIndex(p)->Apply(0, pContext);
		pContext->DrawIndexedInstanced(mIndexNum, mVisibleObjectCount, 0, 0, 0);
	}
}

void InstancedSkull::BuildInstanceBuffer(ID3D11Device* pDevice)
{
	const int n = 10;
	mInstancedData.resize(n*n*n);

	float width = 400.0f;
	float height = 400.0f;
	float depth = 400.0f;

	float x = -0.5f*width;
	float y = -0.5f*height;
	float z = -0.5f*depth;
	float dx = width / (n-1);
	float dy = height / (n-1);
	float dz = depth / (n-1);
	for(int k = 0; k < n; ++k)
	{
		for(int i = 0; i < n; ++i)
		{
			for(int j = 0; j < n; ++j)
			{
				// Position instanced along a 3D grid.
				mInstancedData[k*n*n + i*n + j].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x+j*dx, y+i*dy, z+k*dz, 1.0f);

				// Random color.
				mInstancedData[k*n*n + i*n + j].Color.x = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i*n + j].Color.y = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i*n + j].Color.z = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[k*n*n + i*n + j].Color.w = 1.0f;
			}
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vbData;
	vbData.pSysMem = &mInstancedData[0];

	HR(pDevice->CreateBuffer(&vbd, &vbData, &mInstanceVB));
}