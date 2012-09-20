

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
	
	float h=0.0;
	float dx=0.0;
	float dz=0.0;

	//for (float i=0.125;i<1025;i*=2)
	for (float i=16;i<300;i*=2)
	{
		float u;
		float v;
		sincos(i,u,v);
		h += 0.1*i*(sin((u*output.position.x + v*output.position.z)/i) + cos((v*output.position.x + u*output.position.z)/i));
		output.position.y += h;

		dx +=0.1*(u*cos((v*output.position.z+u*output.position.x)/i)-v*sin((u*output.position.z+v*output.position.x)/i));
		dz +=0.1*(v*cos((v*output.position.z+u*output.position.x)/i)-u*sin((u*output.position.z+v*output.position.x)/i));
	}

	output.position.y += h;

	output.normal = normalize(cross(float3(0.0,dz,0.1),float3(0.1,dx,0.0)));

	output.diffuse=output.position*0.001+float3(0.2,0.2,0.2);

    return output;
}
