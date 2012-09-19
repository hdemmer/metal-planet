
//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float2 position : POSITION;
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
	
	float h = sin(input.position.x) + cos(input.position.y);

    output.position.xz = input.position.xy + float2(-5.0,-5.0);
	output.position.z = output.position.z * 0.2;
    output.position.y = h;

	output.normal = float3(0,1,0);
	output.diffuse=output.position*0.1+float3(0.2,0.2,0.2);

    return output;
}
