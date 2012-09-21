

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
	float3 specular : SPECULAR;
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

	for (float i=32;i<1025;i*=1.8)
	{
		float u;
		float v;
		sincos(i,u,v);
		h += 0.01*sqrt(i)*i*(sin((u*output.position.x + v*output.position.z)/i) + cos((v*output.position.x + u*output.position.z)/i));
		output.position.y += h;

		dx +=0.01*sqrt(i)*(u*cos((v*output.position.z+u*output.position.x)/i)-v*sin((u*output.position.z+v*output.position.x)/i));
		dz +=0.01*sqrt(i)*(v*cos((v*output.position.z+u*output.position.x)/i)-u*sin((u*output.position.z+v*output.position.x)/i));
	}

	output.position.y += h;

	output.normal = normalize(cross(float3(0.0,dz,0.1),float3(0.1,dx,0.0)));

	float disp = saturate(sin(output.position.y/10));

	output.position += output.normal * 10.0 * disp;

	output.diffuse=float3(0.7,0.7,0.7);
	output.specular=float3(0.7,0.7,0.7);

    return output;
}
