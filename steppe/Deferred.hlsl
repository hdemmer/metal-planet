
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

	output.normal = input.position;
	output.diffuse = input.diffuse;

    return output;
}

float4 DeferredPixelShader(PixelInputType input) : SV_TARGET
{
    float4 diffuse;
	diffuse.xyz=input.diffuse;
	diffuse.w=1.0;
    return diffuse;
}

////////////////////////////////////////////////////////////////////////////////
// Lighting pass
////////////////////////////////////////////////////////////////////////////////

struct LightingPixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


SamplerState pointSampler;
Texture2D diffuseTexture;
Texture2D normalTexture;

float4 DeferredLightingPixelShader(LightingPixelInputType input) : SV_TARGET
{
    return diffuseTexture.Sample(pointSampler, input.texCoord);
}



