#include "SkinnedEffect.h"
#include "d3dApp.h"

const D3D11_INPUT_ELEMENT_DESC PosNormalTexTanSkinnedInputLayoutDesc[6] =
{
	{"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"WEIGHTS",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BONEINDICES",  0, DXGI_FORMAT_R8G8B8A8_UINT,   0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

class PosNormalTexTanSkinnedInputLayout :public ResourceInstance< PosNormalTexTanSkinnedInputLayout, ID3D11InputLayout >
{
public:
	PosNormalTexTanSkinnedInputLayout(ID3D11Device* device)
	{
		D3DX11_PASS_DESC passDesc;

		SkinnedEffect::Ptr()->Light1SkinnedTech->GetPassByIndex(0)->GetDesc(&passDesc);
		HR(device->CreateInputLayout(PosNormalTexTanSkinnedInputLayoutDesc, 6, passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize, &m_pResource));
	}
};
ID3D11InputLayout* PosNormalTexTanSkinned::GetInputLayout()
{
	return PosNormalTexTanSkinnedInputLayout::Ptr();
}

SkinnedEffect::SkinnedEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1SkinnedTech = mFX->GetTechniqueByName("Light1Skinned");
	Light2SkinnedTech = mFX->GetTechniqueByName("Light2Skinned");
	Light3SkinnedTech = mFX->GetTechniqueByName("Light3Skinned");

	Light0TexSkinnedTech = mFX->GetTechniqueByName("Light0TexSkinned");
	Light1TexSkinnedTech = mFX->GetTechniqueByName("Light1TexSkinned");
	Light2TexSkinnedTech = mFX->GetTechniqueByName("Light2TexSkinned");
	Light3TexSkinnedTech = mFX->GetTechniqueByName("Light3TexSkinned");

	Light0TexAlphaClipSkinnedTech = mFX->GetTechniqueByName("Light0TexAlphaClipSkinned");
	Light1TexAlphaClipSkinnedTech = mFX->GetTechniqueByName("Light1TexAlphaClipSkinned");
	Light2TexAlphaClipSkinnedTech = mFX->GetTechniqueByName("Light2TexAlphaClipSkinned");
	Light3TexAlphaClipSkinnedTech = mFX->GetTechniqueByName("Light3TexAlphaClipSkinned");

	Light1FogSkinnedTech = mFX->GetTechniqueByName("Light1FogSkinned");
	Light2FogSkinnedTech = mFX->GetTechniqueByName("Light2FogSkinned");
	Light3FogSkinnedTech = mFX->GetTechniqueByName("Light3FogSkinned");

	Light0TexFogSkinnedTech = mFX->GetTechniqueByName("Light0TexFogSkinned");
	Light1TexFogSkinnedTech = mFX->GetTechniqueByName("Light1TexFogSkinned");
	Light2TexFogSkinnedTech = mFX->GetTechniqueByName("Light2TexFogSkinned");
	Light3TexFogSkinnedTech = mFX->GetTechniqueByName("Light3TexFogSkinned");

	Light0TexAlphaClipFogSkinnedTech = mFX->GetTechniqueByName("Light0TexAlphaClipFogSkinned");
	Light1TexAlphaClipFogSkinnedTech = mFX->GetTechniqueByName("Light1TexAlphaClipFogSkinned");
	Light2TexAlphaClipFogSkinnedTech = mFX->GetTechniqueByName("Light2TexAlphaClipFogSkinned");
	Light3TexAlphaClipFogSkinnedTech = mFX->GetTechniqueByName("Light3TexAlphaClipFogSkinned");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	WorldViewProjTex = mFX->GetVariableByName("gWorldViewProjTex")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	BoneTransforms = mFX->GetVariableByName("gBoneTransforms")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
}

SkinnedEffect::~SkinnedEffect()
{
}