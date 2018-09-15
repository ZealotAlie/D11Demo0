#include "RenderObject.h"
#include "UnknownInstance\Basic32.h"

RenderObject::~RenderObject()
{
	ReleaseCOM(mTextureSRV);
	ReleaseCOM(mVertexBuffer);
	ReleaseCOM(mIndexBuffer);
}

void RenderObject::Draw(const D3DApp* pApp)
{
	ID3DX11EffectTechnique* pTech = BasicEffect::Ptr()->GetTechnique(pApp->GetRenderOptions());
	ID3D11DeviceContext* pD3dContext = pApp->GetContext();

	SetupEffect( BasicEffect::Ptr(), pApp );

	DrawWithTech( pD3dContext, pTech );
}

void RenderObject::DrawWithTech( ID3D11DeviceContext* pD3dContext,ID3DX11EffectTechnique* pTech )const
{
	pD3dContext->IASetInputLayout(Basic32::GetInputLayout());
	pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Basic32);
	UINT offset = 0;
	pD3dContext->IASetVertexBuffers( 0, 1, &mVertexBuffer,&stride,&offset );
	pD3dContext->IASetIndexBuffer(mIndexBuffer,DXGI_FORMAT_R32_UINT,0);

	D3DX11_TECHNIQUE_DESC techDesc;
	pTech->GetDesc(&techDesc);

	for(UINT p=0;p<techDesc.Passes;++p)
	{
		pTech->GetPassByIndex(p)->Apply(0,pD3dContext);
		pD3dContext->DrawIndexed(mIndexNum,0,0);
	}
}