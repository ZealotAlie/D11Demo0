#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include "../d3dUtil.h"
#include "UnknownInstance.h"

//////////////////////////////////////////////////////////////////////////
//Rasterizer State
//////////////////////////////////////////////////////////////////////////

class RSWireframe :public ResourceInstance< RSWireframe, ID3D11RasterizerState >
{
public:
	RSWireframe(ID3D11Device* device);
};

class RSNoCull :public ResourceInstance< RSNoCull, ID3D11RasterizerState >
{
public:
	RSNoCull(ID3D11Device* device);
};

class RSCullClockwise :public ResourceInstance< RSCullClockwise, ID3D11RasterizerState >
{
public:
	RSCullClockwise(ID3D11Device* device);
};

//////////////////////////////////////////////////////////////////////////
//Blend State
//////////////////////////////////////////////////////////////////////////
class BSAlphaToCoverage :public ResourceInstance< BSAlphaToCoverage, ID3D11BlendState >
{
public:
	BSAlphaToCoverage(ID3D11Device* device);
};

class BSTransparent :public ResourceInstance< BSTransparent, ID3D11BlendState >
{
public:
	BSTransparent(ID3D11Device* device);
};

class BSNoRenderTargetWrites :public ResourceInstance< BSNoRenderTargetWrites, ID3D11BlendState >
{
public:
	BSNoRenderTargetWrites(ID3D11Device* device);
};

//////////////////////////////////////////////////////////////////////////
//Depth Stencil State
//////////////////////////////////////////////////////////////////////////
class DDSMarkMirror :public ResourceInstance< DDSMarkMirror, ID3D11DepthStencilState >
{
public:
	DDSMarkMirror(ID3D11Device* device);
};

class DSSDrawReflection :public ResourceInstance< DSSDrawReflection, ID3D11DepthStencilState >
{
public:
	DSSDrawReflection(ID3D11Device* device);
};

class DSSStencilNoDoubleBlend :public ResourceInstance< DSSStencilNoDoubleBlend, ID3D11DepthStencilState >
{
public:
	DSSStencilNoDoubleBlend(ID3D11Device* device);
};

class DSSNoDepth :public ResourceInstance< DSSNoDepth, ID3D11DepthStencilState >
{
public:
	DSSNoDepth(ID3D11Device* device);
};

#endif // RENDERSTATES_H