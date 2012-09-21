
cbuffer deferredConstantsBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
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
};

////////////////////////////////////////////////////////////////////////////////
// Deferred pass
////////////////////////////////////////////////////////////////////////////////

struct PixelInputType
{
    float4 position : SV_POSITION;
	float3 diffuse : DIFFUSE;
	float3 normal : NORMAL;
	float3 worldPosition : WORLD_POSITION;
};

struct PixelOutputType
{
    float4 diffuse : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 worldPosition : SV_TARGET2;
};

PixelInputType DeferredVertexShader(VertexInputType input)
{
    PixelInputType output;

	output.worldPosition = input.position;
    
	output.position = mul(float4(input.position,1.0), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	//float4 transformedNormal = mul(float4(input.normal,1.0), viewMatrix);

	output.normal = input.normal;
	output.diffuse = input.diffuse;

    return output;
}

PixelOutputType DeferredPixelShader(PixelInputType input)
{
	PixelOutputType output;
	output.diffuse=float4(input.diffuse,1.0);
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
	
	position = mul(float4(input.position,1.0), worldMatrix);
	position = mul(position, viewMatrix);
	position = mul(position, projectionMatrix);

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
Texture2D diffuseTexture  : register(t0);
Texture2D normalTexture : register(t1);
Texture2D worldPositionTexture : register(t2);

float4 DeferredLightingPixelShader(LightingPixelInputType input) : SV_TARGET
{
	float3 lightPosition = float3(3000,200,3000);

	float4 worldPositionSample = worldPositionTexture.Sample(pointSampler, input.texCoord);

	float3 worldPosition=worldPositionSample.xyz;

	float3 normal = normalTexture.Sample(pointSampler, input.texCoord).xyz;

// calculate blinn-phong

	float3 blinnHalf = normalize( playerEyePosition-worldPosition+lightPosition-worldPosition );
	float diffuseIntensity = saturate( dot(normal,normalize(lightPosition-worldPosition)));
	float specularIntensity = pow( saturate(dot(normal,blinnHalf)), 40 );

	float4 result = diffuseTexture.Sample(pointSampler, input.texCoord)*diffuseIntensity + float4(1.0,1.0,1.0,1.0) * specularIntensity;

    return result;
}



