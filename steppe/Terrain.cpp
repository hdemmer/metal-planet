


#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>


extern ID3D11DeviceContext *devcon;
extern ID3D11Device *dev;

extern void OutputShaderErrorMessage(ID3D10Blob* errorMessage);


ID3D11Buffer * terrainVertexBuffer;
ID3D11VertexShader * terrainVertexShader;
ID3D11InputLayout * terrainInputLayout;

ID3D11PixelShader * terrainPixelShader;

struct TerrainVertexType
	{
		D3DXVECTOR2 position;
	};


void SetupTerrain()
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	int vertexCount = 6;
	TerrainVertexType * vertices = (TerrainVertexType*)malloc(sizeof(TerrainVertexType)*vertexCount);

	vertices[0].position=D3DXVECTOR2(-1,1);
	vertices[1].position=D3DXVECTOR2(1,1);
	vertices[2].position=D3DXVECTOR2(1,-1);
	vertices[3].position=D3DXVECTOR2(1,-1);
	vertices[4].position=D3DXVECTOR2(-1,-1);
	vertices[5].position=D3DXVECTOR2(-1,1);

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

	vertexShaderBlob->Release();

	// compile pixel shader

	ID3D10Blob* pixelShaderBlob = NULL;

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
	terrainVertexShader->Release();
	terrainInputLayout->Release();

	terrainPixelShader->Release();
}

void RenderTerrain()
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(TerrainVertexType); 
	offset = 0;

	devcon->IASetInputLayout(terrainInputLayout);
	devcon->IASetVertexBuffers(0,1,&terrainVertexBuffer,&stride,&offset);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devcon->VSSetShader(terrainVertexShader,NULL,0);
	devcon->PSSetShader(terrainPixelShader,NULL,0);

	devcon->Draw(6,0);
}


