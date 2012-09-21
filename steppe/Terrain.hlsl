

cbuffer terrainConstantsBuffer
{
    float2 origin;
	int octave;
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

SamplerState linearSampler;
Texture2D bumpTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D diffuseTexture : register(t2);
Texture2D specularTexture : register(t3);

DeferredVertexInputType TerrainVertexShader(VertexInputType input)
{
    DeferredVertexInputType output;
	
	float scale = 1.0 / (1 << octave);
	int mipLevel = 7-octave;

	output.position.xzy = input.position*scale + float3(origin,0.0);
	
	// sinus waves

	float h=0.0;
	float dx=0.0;
	float dz=0.0;
	
	for (float i=64;i<1025;i*=1.8)
	{
		float u;
		float v;
		sincos(i,u,v);
		h += 0.01*sqrt(i)*i*(sin((u*output.position.x + v*output.position.z)/i) + cos((v*output.position.x + u*output.position.z)/i));
		output.position.y += h;

		dx +=0.01*sqrt(i)*(u*cos((v*output.position.z+u*output.position.x)/i)-v*sin((u*output.position.z+v*output.position.x)/i));
		dz +=0.01*sqrt(i)*(v*cos((v*output.position.z+u*output.position.x)/i)-u*sin((u*output.position.z+v*output.position.x)/i));
	}
	
	output.position.y += h*0.1;

	float3 tangentX = normalize(float3(1.0,dx,0.0));
	float3 tangentZ = normalize(float3(0.0,dz,1.0));
	
	output.normal = cross(tangentZ,tangentX);

	// texcoords

	float2 texCoords = output.position.xz / 2000.0;

	// texture based displace

	float4 dispSample = bumpTexture.SampleLevel(linearSampler,texCoords,mipLevel);
	float disp = dispSample.x;

	output.position += output.normal * 400.0 * disp;

	float4 normalSample = normalTexture.SampleLevel(linearSampler,texCoords,mipLevel);

	float normalX = 2.0*normalSample.x - 1.0;
	float normalY = 2.0*normalSample.y - 1.0;
	float normalZ = 2.0*normalSample.z - 1.0;
	output.normal = normalize(normalZ * output.normal + normalX * tangentX + normalY * tangentZ);

	output.diffuse=float3(0.2,0.2,0.2);
	output.specular=float3(0.7,0.7,0.7);

    return output;
}
