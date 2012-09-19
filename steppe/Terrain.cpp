
#include "Globals.h"
#include "Terrain.h"


ID3D11Buffer * terrainVertexBuffer;

ID3D11Buffer * terrainStreamOutBuffer;

ID3D11VertexShader * terrainVertexShader;
ID3D11InputLayout * terrainInputLayout;

ID3D11GeometryShader * terrainDummyGS;

struct TerrainVertexType
	{
		D3DXVECTOR2 position;
};

#define GRID_SIZE 10

void PrepareTerrain()
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(DeferredVertexType); 
	offset = 0;

	devcon->IASetInputLayout(terrainInputLayout);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->IASetVertexBuffers(0,1,&terrainVertexBuffer,&stride,&offset);

	devcon->VSSetShader(terrainVertexShader, NULL,0);
	devcon->GSSetShader(terrainDummyGS, NULL, 0);

	devcon->SOSetTargets(1,&terrainStreamOutBuffer,&offset);

	devcon->Draw(GRID_SIZE*GRID_SIZE,0);

	ID3D11Buffer * pBuf = NULL;
	devcon->SOSetTargets(1,&pBuf,&offset);
}

#include <stdio.h>

void SetupTerrain()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	int vertexCount = GRID_SIZE*GRID_SIZE*6;
	TerrainVertexType * vertices = (TerrainVertexType*)malloc(sizeof(TerrainVertexType)*vertexCount);

	for (int i=0;i<GRID_SIZE;i++)
	{
	for (int j=0;j<GRID_SIZE;j++)
	{
		int baseIndex = 6*(i+j*GRID_SIZE);
		vertices[baseIndex+0].position=D3DXVECTOR2(i,j);
		vertices[baseIndex+1].position=D3DXVECTOR2(i,j+1);
		vertices[baseIndex+2].position=D3DXVECTOR2(i+1,j);
		vertices[baseIndex+3].position=D3DXVECTOR2(i+1,j);
		vertices[baseIndex+4].position=D3DXVECTOR2(i,j+1);
		vertices[baseIndex+5].position=D3DXVECTOR2(i+1,j+1);
	}
	}
	

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(TerrainVertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	dev->CreateBuffer(&vertexBufferDesc, &vertexData, &terrainVertexBuffer);

	free(vertices);

	D3D11_BUFFER_DESC streamOutBufferDesc;

	streamOutBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	streamOutBufferDesc.ByteWidth = sizeof(DeferredVertexType) * GRID_SIZE*GRID_SIZE*6;
	streamOutBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	streamOutBufferDesc.CPUAccessFlags = 0;
	streamOutBufferDesc.MiscFlags = 0;
	streamOutBufferDesc.StructureByteStride = 0;

	dev->CreateBuffer(&streamOutBufferDesc, NULL, &terrainStreamOutBuffer);

	ID3D10Blob* errorMessage = NULL;
	ID3D10Blob* vertexShaderBlob = NULL;

	D3DX11CompileFromFile(L"Terrain.hlsl", NULL, NULL, "TerrainVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
				       &vertexShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &terrainVertexShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, 
          D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

	UINT numElements = sizeof(inputLayout) / sizeof(inputLayout[0]);

	// Create the vertex input layout.
	dev->CreateInputLayout(inputLayout, numElements, vertexShaderBlob->GetBufferPointer(), 
		vertexShaderBlob->GetBufferSize(), &terrainInputLayout);

	vertexShaderBlob->Release();

	ID3D10Blob* gsBlob = NULL;

	D3DX11CompileFromFile(L"Terrain.hlsl", NULL, NULL, "DummyGeometryShader", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
				       &gsBlob, &errorMessage, NULL);
	
	D3D11_SO_DECLARATION_ENTRY soDecl[] = 
{
	{ 0, "POSITION", 0, 0, 3, 0 }
	, { 0, "NORMAL", 0, 0, 3, 0 }
	, { 0, "DIFFUSE", 0, 0, 3, 0 }
};

	UINT stride[1] = {9 * sizeof(float)}; // *NOT* sizeof the above array!
	UINT elems = sizeof(soDecl) / sizeof(D3D11_SO_DECLARATION_ENTRY);

	HRESULT res = dev->CreateGeometryShaderWithStreamOutput(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(),soDecl,elems,stride,1,D3D11_SO_NO_RASTERIZED_STREAM,NULL, &terrainDummyGS);
}


void TearDownTerrain()
{
	terrainVertexBuffer->Release();
	terrainVertexShader->Release();
	terrainInputLayout->Release();

	terrainStreamOutBuffer->Release();
	terrainDummyGS->Release();
}



void RenderTerrain()
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(DeferredVertexType); 
	offset = 0;

	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->IASetVertexBuffers(0,1,&terrainStreamOutBuffer,&stride,&offset);

	// Draw
	devcon->DrawAuto();
}


