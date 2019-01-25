#include "TerrainEffect.h"
#include "d3dApp.h"

const D3D11_INPUT_ELEMENT_DESC TerrainVertexInputLayoutDesc[3] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

class TerrainVertexInputLayout:public ResourceInstance< TerrainVertexInputLayout, ID3D11InputLayout >
{
public:
	TerrainVertexInputLayout(ID3D11Device* device)
	{
		D3DX11_PASS_DESC passDesc;

		TerrainEffect::Ptr()->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
		HR(device->CreateInputLayout(TerrainVertexInputLayoutDesc, 3, passDesc.pIAInputSignature, 
			passDesc.IAInputSignatureSize, &m_pResource));
	}
};

ID3D11InputLayout* TerrainVertex::GetInputLayout()
{
	return TerrainVertexInputLayout::Ptr();
}

TerrainEffect::TerrainEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech    = mFX->GetTechniqueByName("Light1");
	Light2Tech    = mFX->GetTechniqueByName("Light2");
	Light3Tech    = mFX->GetTechniqueByName("Light3");
	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	ViewProj           = mFX->GetVariableByName("gViewProj")->AsMatrix();
	EyePosW            = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor           = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart           = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange           = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights          = mFX->GetVariableByName("gDirLights");
	Mat                = mFX->GetVariableByName("gMaterial");

	MinDist            = mFX->GetVariableByName("gMinDist")->AsScalar();
	MaxDist            = mFX->GetVariableByName("gMaxDist")->AsScalar();
	MinTess            = mFX->GetVariableByName("gMinTess")->AsScalar();
	MaxTess            = mFX->GetVariableByName("gMaxTess")->AsScalar();
	TexelCellSpaceU    = mFX->GetVariableByName("gTexelCellSpaceU")->AsScalar();
	TexelCellSpaceV    = mFX->GetVariableByName("gTexelCellSpaceV")->AsScalar();
	WorldCellSpace     = mFX->GetVariableByName("gWorldCellSpace")->AsScalar();
	WorldFrustumPlanes = mFX->GetVariableByName("gWorldFrustumPlanes")->AsVector();

	LayerMapArray      = mFX->GetVariableByName("gLayerMapArray")->AsShaderResource();
	BlendMap           = mFX->GetVariableByName("gBlendMap")->AsShaderResource();
	HeightMap          = mFX->GetVariableByName("gHeightMap")->AsShaderResource();
}

TerrainEffect::~TerrainEffect()
{
}