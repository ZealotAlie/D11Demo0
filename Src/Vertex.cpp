#include "Vertex.h"
#include "d3dx11effect.h"
#include "Effects.h"

#pragma region InputLayoutDesc

static const D3D11_INPUT_ELEMENT_DESC Basic32LayoutDesc[3] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

static const D3D11_INPUT_ELEMENT_DESC TreePointLayoutDesc[2] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

#pragma endregion


ID3D11InputLayout* InputLayouts::Vertex = nullptr;
ID3D11InputLayout* InputLayouts::TreePointSprite = nullptr;

//https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-id3d11device-createinputlayout
void InputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	Effects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(Basic32LayoutDesc,3,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&Vertex));

	Effects::TreeSprite->Light3Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(TreePointLayoutDesc,2,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&TreePointSprite));
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(Vertex);
	ReleaseCOM(TreePointSprite);
}