#include "InstancedData.h"
#include "UnknownInstance/UnknownInstance.h"
#include "InstancingEffect.h"

static const D3D11_INPUT_ELEMENT_DESC InstancedBasic32LayoutDesc[] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0},

	{"WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,	D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 16,	D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 32,	D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 48,	D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 64,	D3D11_INPUT_PER_INSTANCE_DATA, 1}
};

class InstancedBasic32Layout:public ResourceInstance< InstancedBasic32Layout, ID3D11InputLayout >
{
public:
	InstancedBasic32Layout(ID3D11Device* device)
	{
		D3DX11_PASS_DESC passDesc;

		InstancingEffect::Ptr()->Light3Tech->GetPassByIndex(0)->GetDesc(&passDesc);
		HR( device->CreateInputLayout( 
			InstancedBasic32LayoutDesc,
			ARRAYSIZE(InstancedBasic32LayoutDesc),
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize, 
			&m_pResource) ) ;
	}
};

ID3D11InputLayout* InstancedData::GetInputLayout()
{
	return InstancedBasic32Layout::Ptr();
}