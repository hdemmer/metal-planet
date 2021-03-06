#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

#include <xnamath.h>

#include <stdio.h>

extern ID3D11DeviceContext *devcon;
extern ID3D11Device *dev;

extern void OutputShaderErrorMessage(ID3D10Blob* errorMessage);

// define the screen resolution
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

#define WORLD_SIZE 10000

extern float gGameTime;
extern float gTimeSinceLastUpdate;

struct DeferredVertexType
{
	XMFLOAT3 position;
	XMFLOAT3 tangentU;
	XMFLOAT3 tangentV;
	XMFLOAT2 texCoords;
};

extern HWND ghWnd;
extern HINSTANCE ghInstance;

