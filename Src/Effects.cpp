#include "Effects.h"
#include <d3dcompiler.h>

Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DCompileFromFile(filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, 0, "fx_5_0", shaderFlags, 
		0, &compiledShader, &compilationMsgs);

	if (!SUCCEEDED(hr))
	{
		Assert(false, compilationMsgs != nullptr ? (char*)compilationMsgs->GetBufferPointer() : "D3DX11CompileFromFile Error");
	}
	else if(compilationMsgs != nullptr)
	{
		OutputDebugStringA((char*)compilationMsgs->GetBufferPointer());
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(),
		compiledShader->GetBufferSize(),0,device,&mFX));
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}

BasicEffect* Effects::BasicFX = nullptr;
TreeSpriteEffect* Effects::TreeSprite = nullptr;
BlurEffect* Effects::BlurFX = nullptr;
WaterEffect* Effects::Water = nullptr;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX = new BasicEffect(device, L"FX/Basic.fx");
	TreeSprite = new TreeSpriteEffect(device,L"FX/TreeSprite.fx");
	BlurFX = new BlurEffect(device,L"FX/Blur.fx");
	Water = new WaterEffect(device,L"FX/Water.fx");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(TreeSprite);
	SafeDelete(BlurFX);
	SafeDelete(Water);
}

BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech    = mFX->GetTechniqueByName("Light1");
	Light2Tech    = mFX->GetTechniqueByName("Light2");
	Light3Tech    = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech    = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech    = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech    = mFX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = mFX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = mFX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

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

BasicEffect::~BasicEffect()
{
}

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