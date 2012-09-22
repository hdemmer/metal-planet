
cbuffer deferredConstantsBuffer
{
	matrix worldViewProjectionMatrix;
	float4 playerEyePosition;
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
	float4 worldPositionSample = worldPositionTexture.Sample(pointSampler, input.texCoord);

	float3 worldPosition=worldPositionSample.xyz;

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
	float totalLightIntensity=0.0;

	float j = 0;
	for (float i = 0; j < 10; i++)
	{
	if (i >= 10.0)
	{
		i=0;
		j++;
	}

	float3 lightPosition = float3(1000*i,1000,1000*j);

	float3 blinnHalf = normalize( normalize(playerEyePosition.xyz-worldPosition)+normalize(lightPosition-worldPosition ));
	float specularBase =  saturate(dot(normal,blinnHalf));

	float specularIntensity = 0.0;
	float specularExponent = specularSample.x;
	float sum=0.0;
	for (float i = 1; i<17; i*=2)
	{
		sum+=1;
		specularIntensity +=pow(specularBase, specularExponent*i);
	}
	specularIntensity/=sum;
	float attenuation = saturate(1000.0/(length(playerEyePosition.xyz-worldPosition)+length(lightPosition-worldPosition )));

	totalLightIntensity += attenuation * specularIntensity * specularSample.y;

	}

	float4 result = float4(float3(1.0,1.0,1.0)*totalLightIntensity,1.0);


	float glowIntensity = pow(saturate( dot(normal,normalize(playerEyePosition.xyz-worldPosition))),5)*5.0;

	result += saturate(1.0-3.0*specularSample.y)*glowIntensity*glowSample;

    return result;
}



