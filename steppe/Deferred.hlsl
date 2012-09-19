
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
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////

PixelInputType DeferredVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
	
    output.position.xyz = input.position;
	output.position.w = 1.0;

	output.position = mul(output.position, modelViewProjectionMatrix);

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

Texture2D shaderTexture;
SamplerState SampleType;

float4 DeferredPixelShader(PixelInputType input) : SV_TARGET
{
    return float4(1.0,1.0,1.0,1.0);
}





