//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType FullScreenQuadVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
	
    output.position.xyz = input.position;
    output.position.w = 1.0;

    // Store the input color for the pixel shader to use.
    output.texCoord = input.texCoord;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

Texture2D shaderTexture;
SamplerState SampleType;

float4 FullScreenQuadPixelShader(PixelInputType input) : SV_TARGET
{
    return shaderTexture.Sample(SampleType, input.texCoord);
}
