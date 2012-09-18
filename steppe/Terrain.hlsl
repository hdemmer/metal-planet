/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix modelViewProjectionMatrix;
};

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
	
    output.position.xz = 0.3 * input.position;
    output.position.y = -0.1;
	output.position.w = 1.0;

	output.position = mul(output.position, modelViewProjectionMatrix);

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
