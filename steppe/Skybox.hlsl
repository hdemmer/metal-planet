
cbuffer deferredConstantsBuffer
{
	matrix worldViewProjectionMatrix;
	matrix galaxyRotation;
	float4 playerEyePosition;
	float4 yawPitchFOV;
	float2 screenSize;
	float gameTime;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD;
};

SamplerState pointSampler;
Texture2D skyboxTexture : register(t0);

float4 SkyboxPixelShader(PixelInputType input) : SV_TARGET
{
	float yaw = -yawPitchFOV.x;
	float pitch = -yawPitchFOV.y;
	float fov = yawPitchFOV.z;

	float PI = 3.1415927;

	float3 viewDir = float3(cos(pitch)*sin(yaw),sin(pitch),cos(pitch)*cos(yaw));
	float3 up = float3(cos(pitch+PI/2)*sin(yaw),sin(pitch+PI/2),cos(pitch+PI/2)*cos(yaw));

	viewDir = mul(viewDir,galaxyRotation);
	up=mul(up,galaxyRotation);

	float3 localX = cross(up,viewDir);
	up = cross(viewDir,localX);

	float fovPerPixel = tan(fov) / sqrt(dot(screenSize,screenSize));

	float x = (input.texCoords.x - 0.5)*2 * fovPerPixel *screenSize.x;
	float y = (input.texCoords.y - 0.5)*2* fovPerPixel *screenSize.y;
	
	float3 fragmentViewDir = normalize(viewDir+localX*x+up*y);

	float u = saturate(atan2(fragmentViewDir.y,fragmentViewDir.z)/3);
	float v = saturate(acos(fragmentViewDir.x)/2-0.25);
	v= saturate(0.5+0.3*fragmentViewDir.x/fragmentViewDir.y);

	float4 result;
	float3 skyboxSample=skyboxTexture.Sample(pointSampler, float2(u,v)).xyz;

	result = float4(skyboxSample,1);

	//result = float4(fragmentViewDir,1);
	//result = float4(viewDir.xyz,1);
    return result;
}



