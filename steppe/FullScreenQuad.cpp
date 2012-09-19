

#include "Globals.h"
#include "FullScreenQuad.h"

ID3D11Buffer * fullScreenQuadVertexBuffer;
ID3D11VertexShader * fullScreenQuadVertexShader;
ID3D11InputLayout * fullScreenQuadInputLayout;

ID3D11PixelShader * fullScreenQuadPixelShader;

struct FullScreenQuadVertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texCoord;
	};

void SetupRenderFullScreenQuad()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	int vertexCount = 6;
	FullScreenQuadVertexType * vertices = (FullScreenQuadVertexType*)malloc(sizeof(FullScreenQuadVertexType)*vertexCount);

	vertices[0].position=D3DXVECTOR3(-1,1,0);
	vertices[0].texCoord = D3DXVECTOR2(0,0);

	vertices[1].position = D3DXVECTOR3(
		1,1,0);
	vertices[1].texCoord = D3DXVECTOR2(1,0);

	vertices[2].position = D3DXVECTOR3(
		1,-1,0);
	vertices[2].texCoord = D3DXVECTOR2(1,1);
	
	vertices[3].position = D3DXVECTOR3(
		1,-1,0);
	vertices[3].texCoord = D3DXVECTOR2(1,1);

	vertices[4].position = D3DXVECTOR3(
		-1,-1,0);
	vertices[4].texCoord = D3DXVECTOR2(0,1);

	vertices[5].position = D3DXVECTOR3(
		-1,1,0);
	vertices[5].texCoord = D3DXVECTOR2(0,0);
	

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(FullScreenQuadVertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	dev->CreateBuffer(&vertexBufferDesc, &vertexData, &fullScreenQuadVertexBuffer);

	free(vertices);

	ID3D10Blob* errorMessage = NULL;
	ID3D10Blob* vertexShaderBlob = NULL;

	D3DX11CompileFromFile(L"FullScreenQuad.hlsl", NULL, NULL, "FullScreenQuadVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
				       &vertexShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &fullScreenQuadVertexShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, 
          D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

	UINT numElements = sizeof(inputLayout) / sizeof(inputLayout[0]);

	// Create the vertex input layout.
	dev->CreateInputLayout(inputLayout, numElements, vertexShaderBlob->GetBufferPointer(), 
		vertexShaderBlob->GetBufferSize(), &fullScreenQuadInputLayout);

	vertexShaderBlob->Release();

	// compile pixel shader

	ID3D10Blob* pixelShaderBlob = NULL;

	D3DX11CompileFromFile(L"FullScreenQuad.hlsl", NULL, NULL, "FullScreenQuadPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
				       &pixelShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &fullScreenQuadPixelShader);

	pixelShaderBlob->Release();
}

void TearDownRenderFullScreenQuad()
{
	fullScreenQuadVertexBuffer->Release();
	fullScreenQuadVertexShader->Release();
	fullScreenQuadInputLayout->Release();

	fullScreenQuadPixelShader->Release();
}

void RenderFullScreenQuad()
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(FullScreenQuadVertexType); 
	offset = 0;

	devcon->IASetInputLayout(fullScreenQuadInputLayout);
	devcon->IASetVertexBuffers(0,1,&fullScreenQuadVertexBuffer,&stride,&offset);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devcon->VSSetShader(fullScreenQuadVertexShader,NULL,0);
	devcon->PSSetShader(fullScreenQuadPixelShader,NULL,0);

	devcon->Draw(6,0);
}





