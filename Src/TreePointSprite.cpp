#include "TreePointSprite.h"
#include "d3dx11effect.h"
#include "DemoEffects.h"
#include "UnknownInstance\UnknownInstance.h"

static const D3D11_INPUT_ELEMENT_DESC TreePointLayoutDesc[2] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
class TreeInputLayout:public ResourceInstance< TreeInputLayout, ID3D11InputLayout >
{
public:
	TreeInputLayout(ID3D11Device* device)
	{
		D3DX11_PASS_DESC passDesc;

		TreeSpriteEffect::Ptr()->Light3Tech->GetPassByIndex(0)->GetDesc(&passDesc);
		HR(device->CreateInputLayout(TreePointLayoutDesc,2,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&m_pResource));
	}
};

ID3D11InputLayout* TreePointSprite::GetInputLayout()
{
	return TreeInputLayout::Ptr();
}