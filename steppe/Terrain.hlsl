

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

	for (float i=0.125;i<1025;i*=2)
	{
		float u;
		float v;
		sincos(i,u,v);
		float h = 0.1*i*(sin((u*output.position.x + v*output.position.z)/i) + cos((v*output.position.x + u*output.position.z)/i));
		output.position.y += h;
	}

	output.normal = float3(0,1,0);
	output.diffuse=output.position*0.001+float3(0.2,0.2,0.2);

    return output;
}
