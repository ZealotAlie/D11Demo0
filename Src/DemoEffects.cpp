#include "DemoEffects.h"

TreeSpriteEffect::TreeSpriteEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light3Tech                = mFX->GetTechniqueByName("Light3");
	Light3TexAlphaClipTech    = mFX->GetTechniqueByName("Light3TexAlphaClip");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	ViewProj            = mFX->GetVariableByName("gViewProj")->AsMatrix();
	EyePosW             = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor            = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart            = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange            = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights           = mFX->GetVariableByName("gDirLights");
	Mat                 = mFX->GetVariableByName("gMaterial");
	TreeTextureMapArray = mFX->GetVariableByName("gTreeMapArray")->AsShaderResource();
}

TreeSpriteEffect::~TreeSpriteEffect()
{
}

BlurEffect::BlurEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	HorzBlurTech = mFX->GetTechniqueByName("HorzBlur");
	VertBlurTech = mFX->GetTechniqueByName("VertBlur");

	Weights     = mFX->GetVariableByName("gWeights")->AsScalar();
	InputMap    = mFX->GetVariableByName("gInput")->AsShaderResource();
	OutputMap   = mFX->GetVariableByName("gOutput")->AsUnorderedAccessView();
}

BlurEffect::~BlurEffect()
{

}

WaterEffect::WaterEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	WaterTech = mFX->GetTechniqueByName("Water");
	WaterTexTech = mFX->GetTechniqueByName("WaterTex");
	WaterTexFog = mFX->GetTechniqueByName("WaterTexFog");
	WaveMotionTech = mFX->GetTechniqueByName("WaveMotion");
	WaveDisturbTech = mFX->GetTechniqueByName("WaveDisturb");

	WaveOutput		  = mFX->GetVariableByName("gWaveOutput")->AsUnorderedAccessView();
	WavePreY		  = mFX->GetVariableByName("gWavePreY")->AsUnorderedAccessView();
	WaveSize		  = mFX->GetVariableByName("gWaveSize")->AsVector();
	DisturbIndex	  = mFX->GetVariableByName("gDisturbIndex")->AsVector();
	DisturbHeight	  = mFX->GetVariableByName("gDisturbHeight")->AsScalar();

	WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor          = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart          = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange          = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights         = mFX->GetVariableByName("gDirLights");
	Mat               = mFX->GetVariableByName("gMaterial");
	DiffuseMap        = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

WaterEffect::~WaterEffect()
{

}