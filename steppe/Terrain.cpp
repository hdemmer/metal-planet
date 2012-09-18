


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

ID3D11Buffer * matrixBuffer;

struct TerrainVertexType
	{
		D3DXVECTOR2 position;
	};


	struct MatrixBufferType
	{
		D3DXMATRIX modelViewProjection;
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

	// setup constants buffer

	D3D11_BUFFER_DESC matrixBufferDesc;
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	 dev->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
}


void TearDownTerrain()
{
	terrainVertexBuffer->Release();
	terrainVertexShader->Release();
	terrainInputLayout->Release();

	terrainPixelShader->Release();

	matrixBuffer->Release();
}

void RenderTerrain()
{

	/// setup shaders

	unsigned int stride;
	unsigned int offset;

	stride = sizeof(TerrainVertexType); 
	offset = 0;

	D3DXMATRIX projectionMatrix;
	D3DXMATRIX lookAtMatrix;
	D3DXMATRIX modelViewProjectionMatrix;

	// Setup the projection matrix.
	float fieldOfView = (float)D3DX_PI / 4.0f;
	float screenAspect = (float)800 / (float)600;

	// Create the projection matrix for 3D rendering.
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, fieldOfView, screenAspect, 0.1, 1000.0);

	D3DXMatrixLookAtLH(&lookAtMatrix, &D3DXVECTOR3(0,0,-1), &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1,0));

	D3DXMatrixMultiply(&modelViewProjectionMatrix, &lookAtMatrix, &projectionMatrix);

	devcon->IASetInputLayout(terrainInputLayout);
	devcon->IASetVertexBuffers(0,1,&terrainVertexBuffer,&stride,&offset);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devcon->VSSetShader(terrainVertexShader,NULL,0);
	devcon->PSSetShader(terrainPixelShader,NULL,0);

	// setup parameters

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Lock the constant buffer so it can be written to.
	devcon->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->modelViewProjection = modelViewProjectionMatrix;

	// Unlock the constant buffer.
	devcon->Unmap(matrixBuffer, 0);

	devcon->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Draw
	devcon->Draw(6,0);
}


