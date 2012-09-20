

#include "Globals.h"
#include "FullScreenQuad.h"

ID3D11Buffer * fullScreenQuadVertexBuffer;
ID3D11VertexShader * fullScreenQuadVertexShader;
ID3D11InputLayout * fullScreenQuadInputLayout;

struct FullScreenQuadVertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texCoord;
	};

void SetupRenderFullScreenQuad()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	int vertexCount = 6;
	FullScreenQuadVertexType * vertices = (FullScreenQuadVertexType*)malloc(sizeof(FullScreenQuadVertexType)*vertexCount);

	vertices[0].position=XMFLOAT3(-1,1,0);
	vertices[0].texCoord = XMFLOAT2(0,0);

	vertices[1].position = XMFLOAT3(
		1,1,0);
	vertices[1].texCoord = XMFLOAT2(1,0);

	vertices[2].position = XMFLOAT3(
		1,-1,0);
	vertices[2].texCoord = XMFLOAT2(1,1);
	
	vertices[3].position = XMFLOAT3(
		1,-1,0);
	vertices[3].texCoord = XMFLOAT2(1,1);

	vertices[4].position = XMFLOAT3(
		-1,-1,0);
	vertices[4].texCoord = XMFLOAT2(0,1);

	vertices[5].position = XMFLOAT3(
		-1,1,0);
	vertices[5].texCoord = XMFLOAT2(0,0);
	

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
}

void TearDownRenderFullScreenQuad()
{
	fullScreenQuadVertexBuffer->Release();
	fullScreenQuadVertexShader->Release();
	fullScreenQuadInputLayout->Release();
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

	devcon->Draw(6,0);
}





