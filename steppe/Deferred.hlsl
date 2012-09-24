
cbuffer deferredConstantsBuffer
{
	matrix worldViewProjectionMatrix;
	matrix galaxyRotation;
	float4 playerEyePosition;
	float4 yawPitchFOV;
	float2 screenSize;
	float gameTime;
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
	float2 texCoord : TEXCOORD0;
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

	float3 accumulatedLightColor=float3(0.05,0.05,0.05)*specularSample.y; // ambient term

	float3 worldToPlayer = playerEyePosition.xyz-worldPosition;
	
	for (float i = -4; i < 5; i+=0.3)
	{
	
	float3 lightPosition = mul(float4(100*i,-1000,40*i,1),galaxyRotation).xyz;

	float3 lightDir = normalize(lightPosition);
	float3 playerDir = normalize(worldToPlayer);
	float3 reflectedLightDir = 2*dot(lightDir,normal)*normal - lightDir;

	float specularBase = saturate(dot(reflectedLightDir,playerDir));

	specularBase *= pow(saturate(dot(lightDir,normal)),0.01);


	float specularIntensity = 0.5*pow(specularBase,specularExponent);
	//specularIntensity +=5.0*pow(specularBase, specularExponent*20);

	specularIntensity +=pow(specularBase,0.6)*0.01; // diffuse term


	float lightIntensity = 0.1*abs(5-abs(i));
	lightIntensity = pow(lightIntensity,1.5);

	float3 lightColor = lerp(float3(0.72,0.6,0.75),float3(0.92,0.9,0.96),0.25*(4-abs(i)));

	accumulatedLightColor += lightColor * specularIntensity * specularSample.y *lightIntensity;

	}

	float4 result = float4(accumulatedLightColor,1.0);


	float glowIntensity = pow(saturate( dot(normal,normalize(worldToPlayer))),5)*5.0;

	result += saturate(1.0-3.0*specularSample.y)*glowIntensity*glowSample;

    return result;
}



