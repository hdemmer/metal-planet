
cbuffer deferredConstantsBuffer
{
    matrix modelViewProjectionMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float3 position : POSITION;
	float3 normal : NORMAL;
	float3 diffuse : DIFFUSE;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 diffuse : DIFFUSE;
};

struct PixelOutputType
{
    float4 position : SV_TARGET0;
	float3 normal : SV_TARGET1;
	float3 diffuse : SV_TARGET2;
};

////////////////////////////////////////////////////////////////////////////////
// Deferred pass
////////////////////////////////////////////////////////////////////////////////

PixelInputType DeferredVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
	
    output.position.xyz = input.position;
	output.position.w = 1.0;
	output.position = mul(output.position, modelViewProjectionMatrix);

	output.normal = input.normal;
	output.diffuse = input.diffuse;

    return output;
}

PixelOutputType DeferredPixelShader(PixelInputType input)
{
	PixelOutputType output;
	output.position=input.position;
	output.normal=input.normal;
	output.diffuse=input.diffuse;
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Lighting pass
////////////////////////////////////////////////////////////////////////////////

struct LightingPixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};


SamplerState pointSampler;
Texture2D positionTexture  : register(t0);
Texture2D normalTexture : register(t1);
Texture2D diffuseTexture : register(t2);


float4 DeferredLightingPixelShader(LightingPixelInputType input) : SV_TARGET
{
    return positionTexture.Sample(pointSampler, input.texCoord)*0.0+normalTexture.Sample(pointSampler, input.texCoord)+diffuseTexture.Sample(pointSampler, input.texCoord)*0.0;
}



