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
    float4 color : COLOR;
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
    output.color.rg = input.texCoord;
    output.color.b=0.0;
	output.color.a=1.0;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 FullScreenQuadPixelShader(PixelInputType input) : SV_TARGET
{
    return float4(input.color.x,input.color.y,0.0,1.0);
}
