#include "Pos.h"

const D3D11_INPUT_ELEMENT_DESC PosLayoutDesc[1] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

class PosInputLayout:public ResourceInstance< PosInputLayout, ID3D11InputLayout >
{
public:
	PosInputLayout(ID3D11Device* device)
	{
		D3DX11_PASS_DESC passDesc;

		SkyEffect::Ptr()->SkyTech->GetPassByIndex(0)->GetDesc(&passDesc);

		HR(device->CreateInputLayout(PosLayoutDesc,1,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&m_pResource));
	}
};



ID3D11InputLayout* Pos::GetInputLayout()
{
	return PosInputLayout::Ptr();
}

SkyEffect::SkyEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	SkyTech       = mFX->GetTechniqueByName("SkyTech");
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap       = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
}