
#include "Globals.h"
#include "Terrain.h"


ID3D11Buffer * terrainVertexBuffer;

ID3D11Buffer * terrainConstantsBuffer;

#define NUM_TILES 1024

ID3D11Buffer * terrainStreamOutBuffer[NUM_TILES];

ID3D11VertexShader * terrainVertexShader;
ID3D11InputLayout * terrainInputLayout;

ID3D11GeometryShader * terrainDummyGS;

struct TerrainVertexType
{
	D3DXVECTOR2 position;
};

struct TerrainConstantsBufferType
{
	D3DXVECTOR2 origin;
	D3DXVECTOR2 fill;
};

#define TILE_SIZE 10
#define GRID_SIZE 16

void PrepareTerrain()
{
	for(int i=0; i<NUM_TILES; i++)
	{

	unsigned int stride;
	unsigned int offset;

	stride = sizeof(TerrainVertexType); 
	offset = 0;

	devcon->IASetInputLayout(terrainInputLayout);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	devcon->IASetVertexBuffers(0,1,&terrainVertexBuffer,&stride,&offset);

	devcon->VSSetShader(terrainVertexShader, NULL,0);
	devcon->GSSetShader(terrainDummyGS, NULL, 0);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	TerrainConstantsBufferType * dataPtr;
	devcon->Map(terrainConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (TerrainConstantsBufferType*)mappedResource.pData;
	dataPtr->origin = D3DXVECTOR2(-5,-5+TILE_SIZE * i);
	devcon->Unmap(terrainConstantsBuffer, 0);
	devcon->VSSetConstantBuffers(0, 1, &terrainConstantsBuffer);

	devcon->SOSetTargets(1,&terrainStreamOutBuffer[i],&offset);

	devcon->Draw(GRID_SIZE*GRID_SIZE*6,0);

	ID3D11Buffer * pBuf = NULL;
	devcon->SOSetTargets(1,&pBuf,&offset);
	}
}

void SetupTerrain()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	UINT vertexCount = GRID_SIZE*GRID_SIZE*6;
	TerrainVertexType * vertices = (TerrainVertexType*)malloc(sizeof(TerrainVertexType)*vertexCount);

	float scale = TILE_SIZE / (float)GRID_SIZE;

	for (int i=0;i<GRID_SIZE;i++)
	{
		for (int j=0;j<GRID_SIZE;j++)
		{
			int baseIndex = 6*(i+j*GRID_SIZE);

			vertices[baseIndex+0].position=D3DXVECTOR2(scale*i,scale*j);
			vertices[baseIndex+1].position=D3DXVECTOR2(scale*(i+1),scale*j);
			vertices[baseIndex+2].position=D3DXVECTOR2(scale*i,scale*(j+1));

			vertices[baseIndex+3].position=D3DXVECTOR2(scale*i,scale*(j+1));
			vertices[baseIndex+4].position=D3DXVECTOR2(scale*(i+1),scale*(j+1));
			vertices[baseIndex+5].position=D3DXVECTOR2(scale*(i+1),scale*j);

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
	streamOutBufferDesc.ByteWidth = sizeof(DeferredVertexType) *vertexCount;
	streamOutBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	streamOutBufferDesc.CPUAccessFlags = 0;
	streamOutBufferDesc.MiscFlags = 0;
	streamOutBufferDesc.StructureByteStride = 0;

	for (int i=0;i<NUM_TILES;i++)
	{
	dev->CreateBuffer(&streamOutBufferDesc, NULL, &terrainStreamOutBuffer[i]);
	}

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


	D3D11_SO_DECLARATION_ENTRY soDecl[] = 
	{
		{ 0, "POSITION", 0, 0, 3, 0 }
		, { 0, "NORMAL", 0, 0, 3, 0 }
		, { 0, "DIFFUSE", 0, 0, 3, 0 }
	};

	UINT stride[1] = {9 * sizeof(float)}; // *NOT* sizeof the above array!
	UINT elems = sizeof(soDecl) / sizeof(D3D11_SO_DECLARATION_ENTRY);

	HRESULT res = dev->CreateGeometryShaderWithStreamOutput(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),soDecl,elems,stride,1,D3D11_SO_NO_RASTERIZED_STREAM,NULL, &terrainDummyGS);

	vertexShaderBlob->Release();

	// setup constants buffer

	D3D11_BUFFER_DESC matrixBufferDesc;
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(TerrainConstantsBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	dev->CreateBuffer(&matrixBufferDesc, NULL, &terrainConstantsBuffer);
}


void TearDownTerrain()
{
	terrainVertexBuffer->Release();
	terrainVertexShader->Release();
	terrainInputLayout->Release();

	for (int i=0; i<NUM_TILES;i++)
	{
		terrainStreamOutBuffer[i]->Release();
	}
	terrainDummyGS->Release();

	terrainConstantsBuffer->Release();
}



void RenderTerrain()
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(DeferredVertexType); 
	offset = 0;

	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (int i=0; i<NUM_TILES;i++)
	{
		devcon->IASetVertexBuffers(0,1,&terrainStreamOutBuffer[i],&stride,&offset);
		devcon->DrawAuto();
	}
}

#include "TerrainTileManager.h"

void UpdateTerrain()
{
	TerrainTile * allTiles = NULL;
	UINT numTiles = 0;

	TerrainTileManagerAllTiles(&allTiles, &numTiles);

	for (int i = 0; i < numTiles; i++)
	{
		printf("Tile\n");
	}
	printf("\n");
}
