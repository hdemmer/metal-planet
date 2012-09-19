
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

	output.normal = input.position;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////


float4 DeferredPixelShader(PixelInputType input) : SV_TARGET
{
    return float4(input.normal.x*0.5+0.5,input.normal.y*0.5+0.5,0.5,1.0);
}





