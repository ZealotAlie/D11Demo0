#include "ParticleEffect.h"
#include "d3dApp.h"

const D3D11_INPUT_ELEMENT_DESC ParticleInputLayoutDesc[5] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

class ParticleDataInputLayout:public ResourceInstance< ParticleDataInputLayout, ID3D11InputLayout >
{
public:
	ParticleDataInputLayout(ID3D11Device* device)
	{
		D3DX11_PASS_DESC passDesc;

		RainEffect::Ptr()->StreamOutTech->GetPassByIndex(0)->GetDesc(&passDesc);
		HR(device->CreateInputLayout(ParticleInputLayoutDesc, 5, passDesc.pIAInputSignature, 
			passDesc.IAInputSignatureSize, &m_pResource));
	}
};

ID3D11InputLayout* ParticleData::GetInputLayout()
{
	return ParticleDataInputLayout::Ptr();
}


ParticleEffect::ParticleEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	StreamOutTech    = mFX->GetTechniqueByName("StreamOutTech");
	DrawTech         = mFX->GetTechniqueByName("DrawTech");

	ViewProj    = mFX->GetVariableByName("gViewProj")->AsMatrix();
	GameTime    = mFX->GetVariableByName("gGameTime")->AsScalar();
	TimeStep    = mFX->GetVariableByName("gTimeStep")->AsScalar();
	EyePosW     = mFX->GetVariableByName("gEyePosW")->AsVector();
	EmitPosW    = mFX->GetVariableByName("gEmitPosW")->AsVector();
	EmitDirW    = mFX->GetVariableByName("gEmitDirW")->AsVector();
	TexArray    = mFX->GetVariableByName("gTexArray")->AsShaderResource();
	RandomTex   = mFX->GetVariableByName("gRandomTex")->AsShaderResource();
}

ParticleEffect::~ParticleEffect()
{
}