

cbuffer terrainConstantsBuffer
{
    float2 origin;
	float1 scale : SCALE;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float3 position : POSITION;
};

struct DeferredVertexInputType
{
    float3 position : POSITION;
	float3 normal : NORMAL;
	float3 diffuse : DIFFUSE;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
DeferredVertexInputType TerrainVertexShader(VertexInputType input)
{
    DeferredVertexInputType output;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
	
	output.position.xzy = input.position*scale + float3(origin,0.0);

	float h = sin(output.position.x) + cos(output.position.z);
	output.position.y += h;

	output.normal = float3(0,1,0);
	output.diffuse=output.position*0.01+float3(0.2,0.2,0.2);

    return output;
}
