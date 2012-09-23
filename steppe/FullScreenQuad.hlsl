//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
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
