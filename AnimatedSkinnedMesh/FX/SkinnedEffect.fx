//=============================================================================
// NormalMap.fx by Frank Luna (C) 2011 All Rights Reserved.
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
	float4x4 gWorldViewProjTex;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

cbuffer cbSkinned
{
	float4x4 gBoneTransforms[96];
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;
Texture2D gNormalMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
	float4 TangentL : TANGENT;
};

struct SkinnedVertexIn
{
	float3 PosL       : POSITION;
	float3 NormalL    : NORMAL;
	float2 Tex        : TEXCOORD;
	float4 TangentL   : TANGENT;
	float3 Weights    : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
	float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
	float4 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
};

VertexOut SkinnedVS(SkinnedVertexIn vin)
{
    VertexOut vout;

	// Init array or else we get strange warnings about SV_POSITION.
	float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL     = float3(0.0f, 0.0f, 0.0f);
	float3 normalL  = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < 4; ++i)
	{
	    // Assume no nonuniform scaling when transforming normals, so 
		// that we do not have to use the inverse-transpose.

	    posL     += weights[i]*mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL  += weights[i]*mul(vin.NormalL,  (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
		tangentL += weights[i]*mul(vin.TangentL.xyz, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}
 
	// Transform to world space space.
	vout.PosW     = mul(float4(posL, 1.0f), gWorld).xyz;
	vout.NormalW  = mul(normalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = float4(mul(tangentL, (float3x3)gWorld), vin.TangentL.w);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}
 
float4 PS(VertexOut pin, 
          uniform int gLightCount, 
		  uniform bool gUseTexure, 
		  uniform bool gAlphaClip, 
		  uniform bool gFogEnabled) : SV_Target
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
		texColor = gDiffuseMap.Sample( samLinear, pin.Tex );

		if(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.1f);
		}
	}

	//
	// Normal mapping
	//

	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
	 
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
			ComputeDirectionalLight(gMaterial, gDirLights[i], bumpedNormalW, toEye, 
				A, D, S);

			ambient += A;    
			diffuse += D;
			spec    += S;
		}
		   
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

technique11 Light1Skinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, false, false, false ) ) );
    }
}

technique11 Light2Skinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, false, false, false ) ) );
    }
}

technique11 Light3Skinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false ) ) );
    }
}

technique11 Light0TexSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true, false, false ) ) );
    }
}

technique11 Light1TexSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true, false, false ) ) );
    }
}

technique11 Light2TexSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true, false, false ) ) );
    }
}

technique11 Light3TexSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, false, false ) ) );
    }
}

technique11 Light0TexAlphaClipSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true, true, false ) ) );
    }
}

technique11 Light1TexAlphaClipSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true, true, false ) ) );
    }
}

technique11 Light2TexAlphaClipSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true, true, false ) ) );
    }
}

technique11 Light3TexAlphaClipSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, false ) ) );
    }
}

technique11 Light1FogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, false, false, true ) ) );
    }
}

technique11 Light2FogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, false, false, true ) ) );
    }
}

technique11 Light3FogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, true ) ) );
    }
}

technique11 Light0TexFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true, false, true ) ) );
    }
}

technique11 Light1TexFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true, false, true ) ) );
    }
}

technique11 Light2TexFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true, false, true ) ) );
    }
}

technique11 Light3TexFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, false, true ) ) );
    }
}

technique11 Light0TexAlphaClipFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true, true, true ) ) );
    }
}

technique11 Light1TexAlphaClipFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, true, true, true ) ) );
    }
}

technique11 Light2TexAlphaClipFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true, true, true ) ) );
    }
}

technique11 Light3TexAlphaClipFogSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, true ) ) ); 
    }
}