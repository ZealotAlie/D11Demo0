
cbuffer cbSettings
{
	float gWeights[11] = 
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
	};
}

cbuffer cbFixed
{
	static const int gBlurRadius = 5;
}

Texture2D gInput;
RWTexture2D<float4> gOutput;

#define N 256
#define CACHE_SIZE (N+2*gBlurRadius)
groupshared float4 gCache[CACHE_SIZE];

[numthreads(N,1,1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	if(groupThreadID.x < gBlurRadius)
	{
		int x = max(dispatchThreadID.x-gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x,dispatchThreadID.y)];
	}
	if(groupThreadID.x >= N-gBlurRadius)
	{
		int x = min(dispatchThreadID.x+gBlurRadius,gInput.Length.x - 1);
		gCache[groupThreadID.x+2*gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	gCache[groupThreadID.x+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	
	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0,0,0,0);
	[unroll]
	for(int i = -gBlurRadius; i <= gBlurRadius;++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;
		blurColor += gWeights[i+gBlurRadius]*gCache[k];
	}

	gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1,N,1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	if(groupThreadID.y < gBlurRadius)
	{
		int y = max(dispatchThreadID.y-gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x,y)];
	}
	if(groupThreadID.y >= N-gBlurRadius)
	{
		int y = min(dispatchThreadID.y+gBlurRadius,gInput.Length.y - 1);
		gCache[groupThreadID.y+2*gBlurRadius] = gInput[int2(dispatchThreadID.x,y)];
	}

	gCache[groupThreadID.y+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	
	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0,0,0,0);
	[unroll]
	for(int i = -gBlurRadius; i <= gBlurRadius;++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;
		blurColor += gWeights[i+gBlurRadius]*gCache[k];
	}

	gOutput[dispatchThreadID.xy] = blurColor;
}

technique11 HorzBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, HorzBlurCS() ) );
    }
}

technique11 VertBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, VertBlurCS() ) );
    }
}
