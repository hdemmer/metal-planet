
cbuffer deferredConstantsBuffer
{
	matrix worldMatrix;
    matrix viewMatrix;
	matrix projectionMatrix;
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
	
	output.position = mul(float4(input.position,1.0), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	//float4 transformedNormal = mul(float4(input.normal,1.0), viewMatrix);

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
	float3 normal = normalTexture.Sample(pointSampler, input.texCoord);

    return diffuseTexture.Sample(pointSampler, input.texCoord)*saturate(dot(normal,float3(0,1,0)));
}



