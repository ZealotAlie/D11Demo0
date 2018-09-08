//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

cbuffer cbWaveDisturb
{
	uint2 gDisturbIndex;
	float gDisturbHeight;
}

cbuffer cbWave
{
	uint2	gWaveSize;//m,n
};

//|0,...,n-1| |n,...,2n-1|
RWStructuredBuffer<VertexIn> gWaveOutput;
RWBuffer<float> gWavePreY;

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD;
};

#define N 256

[numthreads(1,1,1)]
void DisturbCS()
{
	uint row = gDisturbIndex.x;
	uint col = gDisturbIndex.y;
	int index = row + col * gWaveSize.y;

	float halfMag = 0.5f*gDisturbHeight;

	// Disturb the ijth vertex height and its neighbors.
	gWaveOutput[col * gWaveSize.y + row].PosL.y     += gDisturbHeight;
	gWaveOutput[col * gWaveSize.y + row+1].PosL.y   += halfMag;
	gWaveOutput[col * gWaveSize.y + row-1].PosL.y   += halfMag;
	gWaveOutput[(col+1) * gWaveSize.y + row].PosL.y += halfMag;
	gWaveOutput[(col-1) * gWaveSize.y + row].PosL.y += halfMag;
}

[numthreads(N,1,1)]
void WaveCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint row = dispatchThreadID.x / ( gWaveSize.y - 2 ) + 1;
	uint col = dispatchThreadID.x % ( gWaveSize.y - 2 ) + 1;

	if( row < gWaveSize.x )
	{
		float mK1 = -0.988071561f;
		float mK2 = 1.92901182f;
		float mK3 = 0.0147649236f;
		
		int index = col + row * gWaveSize.y;
		float preY = gWavePreY[index];
		gWavePreY[index] = gWaveOutput[index].PosL.y;
		gWaveOutput[index].PosL.y = 
					mK1*preY +
					mK2*gWaveOutput[index].PosL.y +
					mK3*(gWaveOutput[col + 1 + row * gWaveSize.y].PosL.y + 
					     gWaveOutput[col - 1 + row * gWaveSize.y].PosL.y + 
					     gWaveOutput[col + (row+1) * gWaveSize.y].PosL.y + 
						 gWaveOutput[col + (row-1) * gWaveSize.y].PosL.y);

		float l = gWaveOutput[row*gWaveSize.y+col-1].PosL.y;
		float r = gWaveOutput[row*gWaveSize.y+col+1].PosL.y;
		float t = gWaveOutput[(row-1)*gWaveSize.y+col].PosL.y;
		float b = gWaveOutput[(row+1)*gWaveSize.y+col].PosL.y;

		float3 normal = float3( -r+l, 2.0f*0.8f, b-t );
		gWaveOutput[index].NormalL = normalize(normal);
	}
}

VertexOut VS(VertexIn vin, uint id:SV_VertexID)
{
	VertexOut vout;
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}
 
float4 PS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure, uniform bool gFogEnabled) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );
	}
	 
	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{
		// Start with a sum of zero.
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		// Modulate with late add.
		litColor = texColor*(ambient + diffuse) + spec;
	}

	//
	// Fogging
	//

	if( gFogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 WaveMotion
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, WaveCS() ) );
    }
}

technique11 WaveDisturb
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, DisturbCS() ) );
    }
}

technique11 Water
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false) ) );
    }
}

technique11 WaterTex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, false) ) );
    }
}

technique11 WaterTexFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true) ) );
    }
}