#include "Effect.h"
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
