#include "RenderStates.h"

//////////////////////////////////////////////////////////////////////////
//Rasterizer State
//////////////////////////////////////////////////////////////////////////
RSWireframe::RSWireframe(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &m_pResource));
}

RSNoCull::RSNoCull(ID3D11Device* device)
{
	//
	// NoCullRS
	//
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &m_pResource));
}

RSCullClockwise::RSCullClockwise(ID3D11Device* device)
{
	//
	// CullClockwiseRS
	//

	// Note: Define such that we still cull back faces by making front faces CCW.
	// If we did not cull back faces, then we have to worry about the BackFace
	// property in the D3D11_DEPTH_STENCIL_DESC.
	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&cullClockwiseDesc, &m_pResource));
}

//////////////////////////////////////////////////////////////////////////
//Blend State
//////////////////////////////////////////////////////////////////////////
BSAlphaToCoverage::BSAlphaToCoverage(ID3D11Device* device)
{

	//
	// AlphaToCoverageBS
	//

	D3D11_BLEND_DESC alphaToCoverageDesc = {0};
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&alphaToCoverageDesc, &m_pResource));
}

BSTransparent::BSTransparent(ID3D11Device* device)
{
	//
	// TransparentBS
	//

	D3D11_BLEND_DESC transparentDesc = {0};
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &m_pResource));
}

BSNoRenderTargetWrites::BSNoRenderTargetWrites(ID3D11Device* device)
{
	//
	// NoRenderTargetWritesBS
	//

	D3D11_BLEND_DESC noRenderTargetWritesDesc = {0};
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;

	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable    = false;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&noRenderTargetWritesDesc, &m_pResource));
}

//////////////////////////////////////////////////////////////////////////
//Depth Stencil State
//////////////////////////////////////////////////////////////////////////
DDSMarkMirror::DDSMarkMirror(ID3D11Device* device)
{
	//
	// MarkMirrorDSS
	//
	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable      = false;//Disable the depth clip.
	mirrorDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;
	mirrorDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
	mirrorDesc.StencilEnable    = true;
	mirrorDesc.StencilReadMask  = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

	// We are not rendering back facing polygons, so these settings do not matter.
	mirrorDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&mirrorDesc, &m_pResource));
}

DSSDrawReflection::DSSDrawReflection(ID3D11Device* device)
{
	//
	// DrawReflectionDSS
	//
	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable      = true;
	drawReflectionDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
	drawReflectionDesc.StencilEnable    = true;
	drawReflectionDesc.StencilReadMask  = 0xff;
	drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	// We are not rendering back facing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&drawReflectionDesc, &m_pResource));
}

DSSStencilNoDoubleBlend::DSSStencilNoDoubleBlend(ID3D11Device* device)
{
	//
	// NoDoubleBlendDSS
	//
	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc.DepthEnable      = true;
	noDoubleBlendDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
	noDoubleBlendDesc.StencilEnable    = true;
	noDoubleBlendDesc.StencilReadMask  = 0xff;
	noDoubleBlendDesc.StencilWriteMask = 0xff;

	noDoubleBlendDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
	noDoubleBlendDesc.FrontFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	noDoubleBlendDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
	noDoubleBlendDesc.BackFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&noDoubleBlendDesc, &m_pResource));
}

DSSNoDepth::DSSNoDepth(ID3D11Device* device)
{
	D3D11_DEPTH_STENCIL_DESC noDepthDesc;
	noDepthDesc.DepthEnable      = true;
	noDepthDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;
	noDepthDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
	noDepthDesc.StencilEnable    = false;
	noDepthDesc.StencilReadMask  = 0xff;
	noDepthDesc.StencilWriteMask = 0xff;

	noDepthDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDepthDesc.FrontFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

	// We are not rendering back facing polygons, so these settings do not matter.
	noDepthDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDepthDesc.BackFace.StencilFunc   = D3D11_COMPARISON_EQUAL;
	HR(device->CreateDepthStencilState(&noDepthDesc, &m_pResource));
}