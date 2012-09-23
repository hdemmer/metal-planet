
cbuffer deferredConstantsBuffer
{
	matrix worldViewProjectionMatrix;
	float4 playerEyePosition;
	float4 yawPitchFOV;
	float2 screenSize;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

SamplerState pointSampler;
Texture2D skyboxTexture : register(t0);

float4 SkyboxPixelShader(PixelInputType input) : SV_TARGET
{
	float yaw = yawPitchFOV.x;
	float pitch = yawPitchFOV.y;
	float fov = yawPitchFOV.z;
	

	float u = (-yaw + fov * input.texCoord.x)/1.57;
	float v = saturate((-pitch + fov * input.texCoord.y)*0.5);

	float4 result;
	float3 skyboxSample=skyboxTexture.Sample(pointSampler, float2(v,u)).xyz;

	result = float4(skyboxSample,1);

	//result = float4(u,v,0,1);
    return result;
}



