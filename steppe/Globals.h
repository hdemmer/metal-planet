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
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

struct DeferredVertexType
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 diffuse;
};

extern HWND ghWnd;
extern HINSTANCE ghInstance;

