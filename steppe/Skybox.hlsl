
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
Texture2D galaxyTexture : register(t0);
Texture2D skyboxTexture : register(t1);

float4 SkyboxPixelShader(PixelInputType input) : SV_TARGET
{
	float yaw = -yawPitchFOV.x;
	float pitch = -yawPitchFOV.y;
	float fov = yawPitchFOV.z;

	float PI = 3.1415927;

	float3 viewDir = float3(cos(pitch)*sin(yaw),sin(pitch),cos(pitch)*cos(yaw));
	float3 up = float3(cos(pitch+PI/2)*sin(yaw),sin(pitch+PI/2),cos(pitch+PI/2)*cos(yaw));

	viewDir = mul(float4(viewDir,1),galaxyRotation).xyz;
	up=mul(float4(up,1),galaxyRotation).xyz;

	float3 localX = normalize(cross(up,viewDir));
	up = normalize(cross(viewDir,localX));

	float fovPerPixel = tan(fov) / sqrt(dot(screenSize,screenSize));

	float x = (input.texCoords.x - 0.5)*2 * fovPerPixel *screenSize.x;
	float y = (input.texCoords.y - 0.5)*2* fovPerPixel *screenSize.y;
	
	float3 fragmentViewDir = normalize(viewDir+localX*x+up*y);

	float u,v;

	// galaxy

	u = saturate(0.5+0.5*fragmentViewDir.z/fragmentViewDir.y);
	v = saturate(0.5+0.5*fragmentViewDir.x/fragmentViewDir.y*(1+0.1*cos(atan2(fragmentViewDir.y,fragmentViewDir.z))));

	float4 result;
	float3 galaxySample=galaxyTexture.Sample(pointSampler, float2(v,u)).xyz;

	if (fragmentViewDir.y <= 0.001)
	{
		// reject negative y or we would have two galaxies
		// also remove div by zero artifacts
		galaxySample = float3(0,0,0);
	}

	// stars
	u = fragmentViewDir.x/fragmentViewDir.z;
	v = fragmentViewDir.y/fragmentViewDir.z;
	
	if (abs(u) > 1 || abs(v)>1)
	{
		u = fragmentViewDir.x/fragmentViewDir.y;
		v = fragmentViewDir.z/fragmentViewDir.y;
	}
	if (abs(u) > 1 || abs(v)>1)
	{
		u = fragmentViewDir.y/fragmentViewDir.x;
		v = fragmentViewDir.z/fragmentViewDir.x;
	}

	float3 starsSample=skyboxTexture.Sample(pointSampler, float2(u,v)).xyz;

	result = float4(galaxySample+0.6*starsSample,1);
    return result;
}



