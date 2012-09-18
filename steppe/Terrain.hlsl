//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float2 position : POSITION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType TerrainVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
	
    output.position.xy = 0.5 * input.position;
    output.position.z = 0.0;
	output.position.w=1.0;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

Texture2D shaderTexture;
SamplerState SampleType;

float4 TerrainPixelShader(PixelInputType input) : SV_TARGET
{
    return float4(1.0,1.0,1.0,1.0);
}
