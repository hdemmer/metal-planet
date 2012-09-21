
#include "Globals.h"
#include "Terrain.h"

#include "TerrainTileManager.h"

ID3D11Buffer * terrainVertexBuffer;
ID3D11Buffer * terrainConstantsBuffer;

ID3D11VertexShader * terrainGenerateVertexShader;
ID3D11InputLayout * terrainInputLayout;

ID3D11GeometryShader * terrainDummyGS;

ID3D11Buffer * terrainIndexBuffer;

ID3D11ShaderResourceView* bumpTexture;
ID3D11ShaderResourceView* normalTexture;
ID3D11ShaderResourceView* diffuseTexture;
ID3D11ShaderResourceView* specularTexture;

ID3D11SamplerState* sampleStateLinear;

ID3D11PixelShader * terrainPixelShader;

struct TerrainVertexType
{
	XMFLOAT3 position;
};

struct TerrainConstantsBufferType
{
	XMFLOAT2 origin;
	UINT octave;
	float padding;
};

#define GRID_SIZE 64
#define OVERLAP_WIDTH 2


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

	devcon->VSSetShader(terrainGenerateVertexShader, NULL,0);
	devcon->GSSetShader(terrainDummyGS, NULL, 0);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	TerrainConstantsBufferType * dataPtr;
	devcon->Map(terrainConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (TerrainConstantsBufferType*)mappedResource.pData;
	dataPtr->origin = tile->origin;
	dataPtr->octave = tile->depth;
	devcon->Unmap(terrainConstantsBuffer, 0);
	devcon->VSSetConstantBuffers(0, 1, &terrainConstantsBuffer);

	devcon->VSSetSamplers(0,1,&sampleStateLinear);
	ID3D11ShaderResourceView *textures[3]={bumpTexture,normalTexture,diffuseTexture};
	devcon->VSSetShaderResources(0, 3, textures);

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

	float scale = TILE_BASE_SIZE / (float)(GRID_SIZE - 2* OVERLAP_WIDTH);

	for (int i=0;i<=GRID_SIZE;i++)
	{
		for (int j=0;j<=GRID_SIZE;j++)
		{
			float overlap = 0;
			if (i< OVERLAP_WIDTH)
				overlap+=OVERLAP_WIDTH-i;
			if (i >= GRID_SIZE - OVERLAP_WIDTH)
				overlap+=i-GRID_SIZE + OVERLAP_WIDTH;
			if (j< OVERLAP_WIDTH)
				overlap+=OVERLAP_WIDTH-j;
			if (j >= GRID_SIZE - OVERLAP_WIDTH)
				overlap+=j-GRID_SIZE + OVERLAP_WIDTH;

			if (OVERLAP_WIDTH != 0)
				overlap /= -2*OVERLAP_WIDTH;

			int baseIndex = (i+j*(GRID_SIZE+1));
			vertices[baseIndex].position=XMFLOAT3(scale*(i-OVERLAP_WIDTH),scale*(j-OVERLAP_WIDTH),overlap*1000);
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

	D3DX11CompileFromFile(L"Terrain.hlsl", NULL, NULL, "TerrainGenerateVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
		&vertexShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &terrainGenerateVertexShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
		D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = sizeof(inputLayout) / sizeof(inputLayout[0]);

	// Create the vertex input layout.
	dev->CreateInputLayout(inputLayout, numElements, vertexShaderBlob->GetBufferPointer(), 
		vertexShaderBlob->GetBufferSize(), &terrainInputLayout);


	D3D11_SO_DECLARATION_ENTRY soDecl[] = 
	{
		{ 0, "POSITION", 0, 0, 3, 0 }
		, { 0, "TANGENTU", 0, 0, 3, 0 }
		, { 0, "TANGENTV", 0, 0, 3, 0 }
		, { 0, "TEXCOORDS", 0, 0, 2, 0 }
	};

	UINT stride[1] = {11 * sizeof(float)}; // *NOT* sizeof the above array!
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

	// setup textures

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	dev->CreateSamplerState(&samplerDesc, &sampleStateLinear);

 	res =  D3DX11CreateShaderResourceViewFromFile(dev, L"textures/am_diffuse.png", NULL, NULL, &diffuseTexture, NULL);
	//D3DX11CreateShaderResourceViewFromFile(dev, L"textures/am_specular.png", NULL, NULL, &specularTexture, NULL);
	D3DX11CreateShaderResourceViewFromFile(dev, L"textures/am_bump.png", NULL, NULL, &bumpTexture, NULL);
	D3DX11CreateShaderResourceViewFromFile(dev, L"textures/am_normal.png", NULL, NULL, &normalTexture, NULL);

	///////////////
	// setup  pixel shader

	ID3D10Blob * pixelShaderBlob = NULL;

	D3DX11CompileFromFile(L"Terrain.hlsl", NULL, NULL, "TerrainPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
		&pixelShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &terrainPixelShader);

	pixelShaderBlob->Release();
}


void TearDownTerrain()
{
	terrainVertexBuffer->Release();
	terrainGenerateVertexShader->Release();
	terrainInputLayout->Release();

	terrainDummyGS->Release();

	terrainConstantsBuffer->Release();

	diffuseTexture->Release();
	//specularTexture->Release();
	bumpTexture->Release();
	normalTexture->Release();

	terrainPixelShader->Release();
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
	devcon->PSSetShader(terrainPixelShader,NULL,0);

	devcon->PSSetSamplers(0,1,&sampleStateLinear);
	ID3D11ShaderResourceView *textures[3]={bumpTexture,normalTexture,diffuseTexture};
	devcon->PSSetShaderResources(0, 3, textures);


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
