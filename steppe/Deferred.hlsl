
cbuffer deferredConstantsBuffer
{
	matrix worldViewProjectionMatrix;
	float4 playerEyePosition;
	float4 yawPitchFOV;
	float2 screenSize;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
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
	float4 specular : SV_TARGET3;	// spec exp, spec occ
	float4 glow : SV_TARGET4;
};

////////////////////////////////////////////////////////////////////////////////
// Deferred pass
////////////////////////////////////////////////////////////////////////////////


PixelInputType DeferredVertexShader(VertexInputType input)
{
    PixelInputType output;

	output.worldPosition = input.position;
	output.position = mul(float4(input.position,1.0), worldViewProjectionMatrix);

	output.tangentU = input.tangentU;
	output.tangentV = input.tangentV;
	output.texCoords = input.texCoords;

    return output;
}


PixelOutputType DeferredPixelShader(PixelInputType input)
{
	PixelOutputType output;
	output.diffuse=float4(0.0,0.0,0.0,0.0);
	output.specular=float4(0.0,0.0,0.0,0.0);
	output.glow=float4(0.0,0.0,0.0,0.0);
	output.normal=float4(cross(input.tangentU,input.tangentV),0.0);
	output.worldPosition=float4(input.worldPosition,0.0);
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Lighting pass
////////////////////////////////////////////////////////////////////////////////

struct LightingPixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};


SamplerState pointSampler;
Texture2D worldPositionTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D diffuseTexture  : register(t2);
Texture2D specularTexture  : register(t3);
Texture2D glowTexture  : register(t4);

float4 DeferredLightingPixelShader(LightingPixelInputType input) : SV_TARGET
{
	float3 worldPosition=worldPositionTexture.Sample(pointSampler, input.texCoord).xyz;

	float3 normal = normalTexture.Sample(pointSampler, input.texCoord).xyz;

	float4 specularSample = specularTexture.Sample(pointSampler, input.texCoord);

	float pixelWidth = 1.0/800.0;
	float pixelHeight = 1.0/600.0;

	float4 glowSample = glowTexture.Sample(pointSampler, input.texCoord);
	glowSample *=4;
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(pixelWidth,0));
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(0,pixelHeight));
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(-pixelWidth,0));
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(0,-pixelHeight));
	glowSample*=2;
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(pixelWidth,pixelHeight));
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(-pixelWidth,pixelHeight));
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(-pixelWidth,pixelHeight));
	glowSample += glowTexture.Sample(pointSampler, input.texCoord+float2(-pixelWidth,-pixelHeight));
	glowSample /=16;
//

	float specularExponent = specularSample.x;

	float totalLightIntensity=0.0;

	float3 worldToPlayer = playerEyePosition.xyz-worldPosition;
	
	for (float i = 0; i < 10; i++)
	{
	for (float j = 0; j < 10; j++)
	{
	
	float3 lightPosition = float3(1000*i,1000,1000*j)-worldPosition;

	float3 blinnHalf = normalize( normalize(worldToPlayer)+normalize(lightPosition));
	float specularBase =  saturate(dot(normal,blinnHalf));

	float specularIntensity = pow(specularBase,specularExponent);
	specularIntensity +=pow(specularBase, specularExponent*4);
	specularIntensity +=pow(specularBase, specularExponent*16);
	specularIntensity +=pow(specularBase, specularExponent*32);
	specularIntensity/=4;
	
	float attenuation = saturate(500.0/((length(worldToPlayer)+length(lightPosition))));

	totalLightIntensity += attenuation * specularIntensity * specularSample.y;
	}
	}

	float4 result = float4(float3(1.0,1.0,1.0)*totalLightIntensity,1.0);


	float glowIntensity = pow(saturate( dot(normal,normalize(worldToPlayer))),5)*5.0;

	result += saturate(1.0-3.0*specularSample.y)*glowIntensity*glowSample;

    return result;
}



