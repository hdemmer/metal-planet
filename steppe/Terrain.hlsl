

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
	float3 tangentU : TANGENTU;
	float3 tangentV : TANGENTV;
	float2 texCoords : TEXCOORDS;
};


struct PixelInputType
{
    float4 position : SV_POSITION;
	float3 tangentU : TANGENTU;
	float3 tangentV : TANGENTV;
	float2 texCoords : TEXCOORDS;
	float3 worldPosition : WORLD_POSITION;
};


struct PixelOutputType
{
	float4 worldPosition : SV_TARGET0;
    float4 normal : SV_TARGET1;
	float4 diffuse : SV_TARGET2;
	float4 specular : SV_TARGET3;
};

////////////////////////////////////////////////////////////////////////////////
// Terrain Generation Vertex Shader
////////////////////////////////////////////////////////////////////////////////

SamplerState linearSampler;
Texture2D bumpTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D diffuseTexture : register(t2);
Texture2D specularTexture : register(t3);

DeferredVertexInputType TerrainGenerateVertexShader(VertexInputType input)
{
    DeferredVertexInputType output;
	
	float scale = 1.0 / (1 << octave);
	int mipLevel = 7-octave;

	output.position.xzy = input.position*scale + float3(origin,0.0);
	
	// sinus waves

	float h=0.0;
	float dx=0.0;
	float dz=0.0;
	
	for (float i=64;i<257;i*=1.8)
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

	float3 tangentU = normalize(float3(1.0,dx,0.0));
	float3 tangentV = normalize(float3(0.0,dz,1.0));
	
	output.tangentU = tangentU;
	output.tangentV = tangentV;

	float3 normal = cross(tangentV,tangentU);

	// texcoords

	float2 texCoords = output.position.xz / 5000.0;

	output.texCoords = texCoords;

	// texture based displace



	float disp = 0.0;
	for (float i=1;i<17;i*=4)
	{
		disp += 400.0 * (1.0 / i) * bumpTexture.SampleLevel(linearSampler,texCoords * i,mipLevel).x;
	}

	output.position += normal * disp;

    return output;
}



////////////////////////////////////////////////////////////////////////////////
// Deferred pass
////////////////////////////////////////////////////////////////////////////////


PixelOutputType TerrainPixelShader(PixelInputType input)
{
	PixelOutputType output;
	output.worldPosition=float4(input.worldPosition,0.0);
	output.diffuse=float4(0.1,0.1,0.1,0.0);

	// normal mapping

	float3 normal = cross(input.tangentV, input.tangentU);
	
	float4 normalSample;
	float sum;
	
	for (float i=1;i<65;i*=4)
	{
		sum +=1;
		normalSample += normalTexture.Sample(linearSampler,input.texCoords * i);
	}
	normalSample /= sum;

	float normalX = 2.0 * normalSample.x - 1.0;
	float normalY = 2.0 * normalSample.y - 1.0;
	float normalZ = 2.0 * normalSample.z - 1.0;
	normal = normalize(normalZ * normal + normalX * input.tangentU + normalY * input.tangentV);

	output.normal=float4(normal,0.0);

	float disp = 0.0;
	float sum2;
	for (float i=1;i<17;i*=4)
	{
		sum2+= (1.0/i);
		disp += (1.0 / i) * bumpTexture.Sample(linearSampler,input.texCoords * i).x;
	}

	output.specular=float4(10.0+20.0*disp,1.0,0.0,0.0);

    return output;
}
