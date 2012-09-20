
#include "Globals.h"
#include "Terrain.h"

#include "TerrainTileManager.h"

ID3D11Buffer * terrainVertexBuffer;
ID3D11Buffer * terrainConstantsBuffer;

ID3D11VertexShader * terrainVertexShader;
ID3D11InputLayout * terrainInputLayout;

ID3D11GeometryShader * terrainDummyGS;

ID3D11Buffer * terrainIndexBuffer;

struct TerrainVertexType
{
	XMFLOAT2 position;
};

struct TerrainConstantsBufferType
{
	XMFLOAT2 origin;
	float scale;
	float padding;
};

#define GRID_SIZE 64

void GenerateTerrainTile(TerrainTile*tile)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(TerrainVertexType); 
	offset = 0;

	devcon->IASetInputLayout(terrainInputLayout);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	devcon->IASetVertexBuffers(0,1,&terrainVertexBuffer,&stride,&offset);
	devcon->IASetIndexBuffer( terrainIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	devcon->VSSetShader(terrainVertexShader, NULL,0);
	devcon->GSSetShader(terrainDummyGS, NULL, 0);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	TerrainConstantsBufferType * dataPtr;
	devcon->Map(terrainConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (TerrainConstantsBufferType*)mappedResource.pData;
	dataPtr->origin = tile->origin;
	dataPtr->scale = 1.0f / (float)(1 << tile->depth);
	devcon->Unmap(terrainConstantsBuffer, 0);
	devcon->VSSetConstantBuffers(0, 1, &terrainConstantsBuffer);

	devcon->SOSetTargets(1,&tile->vertexBuffer,&offset);

	devcon->DrawIndexed(GRID_SIZE*GRID_SIZE*6,0,0);

	ID3D11Buffer * pBuf = NULL;
	devcon->SOSetTargets(1,&pBuf,&offset);
}

UINT TerrainExpectedBufferWidth()
{
	return sizeof(DeferredVertexType) * GRID_SIZE*GRID_SIZE * 6;
}

void SetupTerrain()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	UINT vertexCount = (GRID_SIZE+1)*(GRID_SIZE+1);
	TerrainVertexType * vertices = (TerrainVertexType*)malloc(sizeof(TerrainVertexType)*vertexCount);

	float scale = TILE_BASE_SIZE / (float)GRID_SIZE;

	for (int i=0;i<=GRID_SIZE;i++)
	{
		for (int j=0;j<=GRID_SIZE;j++)
		{
			int baseIndex = (i+j*(GRID_SIZE+1));
			vertices[baseIndex].position=XMFLOAT2(scale*i,scale*j);
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



	// Prepare Index buffer

	UINT * indices = (UINT*)malloc(sizeof(UINT)*GRID_SIZE*GRID_SIZE*6);

	for (int i=0;i<GRID_SIZE;i++)
	{
		for (int j=0;j<GRID_SIZE;j++)
		{
			int baseIndex = 6*(i+j*GRID_SIZE);

			indices[baseIndex]=i+j*(GRID_SIZE+1)+1;
			indices[baseIndex+1]=i+j*(GRID_SIZE+1);
			indices[baseIndex+2]=i+(j+1)*(GRID_SIZE+1);

			indices[baseIndex+3]=i+(j+1)*(GRID_SIZE+1);
			indices[baseIndex+4]=i+(j+1)*(GRID_SIZE+1)+1;
			indices[baseIndex+5]=i+1+j*(GRID_SIZE+1);
		}
	}

	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage           = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth       = sizeof( UINT ) *GRID_SIZE*GRID_SIZE * 6;
	bufferDesc.BindFlags       = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags  = 0;
	bufferDesc.MiscFlags       = 0;

	// Define the resource data.
D3D11_SUBRESOURCE_DATA indicesData;
indicesData.pSysMem = indices;
indicesData.SysMemPitch = 0;
indicesData.SysMemSlicePitch = 0;

	dev->CreateBuffer( &bufferDesc, &indicesData, &terrainIndexBuffer );

	free(indices);
}


void TearDownTerrain()
{
	terrainVertexBuffer->Release();
	terrainVertexShader->Release();
	terrainInputLayout->Release();

	terrainDummyGS->Release();

	terrainConstantsBuffer->Release();
}


void RenderTerrainTile(TerrainTile* tile)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(DeferredVertexType); 
	offset = 0;

	devcon->IASetVertexBuffers(0,1,&tile->vertexBuffer,&stride,&offset);
	devcon->DrawAuto();
}

void RenderTerrain()
{
	
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	TerrainTile ** allTiles = NULL;
	UINT numTiles = 0;

	TerrainTileManagerAllLeafTiles(&allTiles, &numTiles);

	for (UINT i = 0; i < numTiles; i++)
	{
		TerrainTile * tile = allTiles[i];
//		printf("Render Tile: %f,%f : %d\n", tile->origin.x, tile->origin.y, tile->depth);

		RenderTerrainTile(tile);
	}

	free(allTiles);
}

void UpdateTerrain()
{
	TerrainTileManagerUpdate();
}
