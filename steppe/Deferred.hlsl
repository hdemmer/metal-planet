
cbuffer deferredConstantsBuffer
{
	matrix worldViewProjectionMatrix;
	float4 playerEyePosition;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float3 position : POSITION;
	float3 normal : NORMAL;
	float3 diffuse : DIFFUSE;
	float3 specular : SPECULAR;	// spec exponent, spec occlusion, ???
};

////////////////////////////////////////////////////////////////////////////////
// Deferred pass
////////////////////////////////////////////////////////////////////////////////

struct PixelInputType
{
    float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 diffuse : DIFFUSE;
	float3 specular : SPECULAR;
	float3 worldPosition : WORLD_POSITION;
};

struct PixelOutputType
{
	float4 worldPosition : SV_TARGET0;
    float4 normal : SV_TARGET1;
	float4 diffuse : SV_TARGET2;
	float4 specular : SV_TARGET3;
};

PixelInputType DeferredVertexShader(VertexInputType input)
{
    PixelInputType output;

	output.worldPosition = input.position;
    
	output.position = mul(float4(input.position,1.0), worldViewProjectionMatrix);
	//output.position = mul(output.position, viewMatrix);
	//output.position = mul(output.position, projectionMatrix);

	//float4 transformedNormal = mul(float4(input.normal,1.0), viewMatrix);

	output.normal = input.normal;
	output.diffuse = input.diffuse;
	output.specular = input.specular;

    return output;
}

PixelOutputType DeferredPixelShader(PixelInputType input)
{
	PixelOutputType output;
	output.diffuse=float4(input.diffuse,1.0);
	output.specular=float4(input.specular,1.0);
	output.normal=float4(input.normal,1.0);
	output.worldPosition=float4(input.worldPosition,0.0);
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Z-Prepass pass
////////////////////////////////////////////////////////////////////////////////


float4 ZPrePassVertexShader(VertexInputType input) : SV_POSITION
{
    float4 position;
	
	position = mul(float4(input.position,1.0), worldViewProjectionMatrix);
	//position = mul(position, viewMatrix);
	//position = mul(position, projectionMatrix);

    return position;
}

float4 ZPrePassPixelShader(float4 position : SV_POSITION) : SV_TARGET
{
	return float4(1.0,1.0,1.0,1.0);
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

float4 DeferredLightingPixelShader(LightingPixelInputType input) : SV_TARGET
{
	float3 lightPosition = float3(5000,10000,5000);

	float4 worldPositionSample = worldPositionTexture.Sample(pointSampler, input.texCoord);

	float3 worldPosition=worldPositionSample.xyz;

	float3 normal = normalTexture.Sample(pointSampler, input.texCoord).xyz;

// calculate blinn-phong

	float3 blinnHalf = normalize( playerEyePosition.xyz-worldPosition+lightPosition-worldPosition );
	float diffuseIntensity = saturate( dot(normal,normalize(lightPosition-worldPosition)));
	float specularIntensity = pow( saturate(dot(normal,blinnHalf)), 50 );

	float4 result =diffuseTexture.Sample(pointSampler, input.texCoord)*diffuseIntensity + float4(1.0,1.0,1.0,1.0) * specularIntensity;

    return result;
}



