#include "SsaoEffect.h"

SsaoEffect::SsaoEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	SsaoTech           = mFX->GetTechniqueByName("Ssao");

	ViewToTexSpace     = mFX->GetVariableByName("gViewToTexSpace")->AsMatrix();
	OffsetVectors      = mFX->GetVariableByName("gOffsetVectors")->AsVector();
	FrustumCorners     = mFX->GetVariableByName("gFrustumCorners")->AsVector();
	OcclusionRadius    = mFX->GetVariableByName("gOcclusionRadius")->AsScalar();
	OcclusionFadeStart = mFX->GetVariableByName("gOcclusionFadeStart")->AsScalar();
	OcclusionFadeEnd   = mFX->GetVariableByName("gOcclusionFadeEnd")->AsScalar();
	SurfaceEpsilon     = mFX->GetVariableByName("gSurfaceEpsilon")->AsScalar();

	NormalDepthMap     = mFX->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	RandomVecMap       = mFX->GetVariableByName("gRandomVecMap")->AsShaderResource();
}

SsaoEffect::~SsaoEffect()
{
}

SsaoBlurEffect::SsaoBlurEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	HorzBlurTech    = mFX->GetTechniqueByName("HorzBlur");
	VertBlurTech    = mFX->GetTechniqueByName("VertBlur");

	TexelWidth      = mFX->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight     = mFX->GetVariableByName("gTexelHeight")->AsScalar();

	NormalDepthMap  = mFX->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	InputImage      = mFX->GetVariableByName("gInputImage")->AsShaderResource();
}

SsaoBlurEffect::~SsaoBlurEffect()
{
}

SsaoNormalDepthEffect::SsaoNormalDepthEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	NormalDepthTech          = mFX->GetTechniqueByName("NormalDepth");
	NormalDepthAlphaClipTech = mFX->GetTechniqueByName("NormalDepthAlphaClip");

	WorldView             = mFX->GetVariableByName("gWorldView")->AsMatrix();
	WorldInvTransposeView = mFX->GetVariableByName("gWorldInvTransposeView")->AsMatrix();
	WorldViewProj         = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	TexTransform          = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	DiffuseMap            = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

SsaoNormalDepthEffect::~SsaoNormalDepthEffect()
{
}