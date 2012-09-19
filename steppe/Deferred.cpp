
#include "Globals.h"

ID3D11Buffer * deferredConstantsBuffer;
ID3D11VertexShader * deferredVertexShader;
ID3D11InputLayout * deferredInputLayout;

ID3D11PixelShader * deferredPixelShader;

struct DeferredVertexType
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 diffuse;
};

struct MatrixBufferType
{
	D3DXMATRIX modelViewProjection;
};


void SetupDeferred()
{

	ID3D10Blob* errorMessage = NULL;
	ID3D10Blob* vertexShaderBlob = NULL;

	D3DX11CompileFromFile(L"Deferred.hlsl", NULL, NULL, "DeferredVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
				       &vertexShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &deferredVertexShader);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
		  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, 
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
		  { "DIFFUSE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, 
          D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

	UINT numElements = sizeof(inputLayout) / sizeof(inputLayout[0]);

	// Create the vertex input layout.
	dev->CreateInputLayout(inputLayout, numElements, vertexShaderBlob->GetBufferPointer(), 
		vertexShaderBlob->GetBufferSize(), &deferredInputLayout);

	vertexShaderBlob->Release();

	// compile pixel shader

	ID3D10Blob* pixelShaderBlob = NULL;

	D3DX11CompileFromFile(L"Deferred.hlsl", NULL, NULL, "DeferredPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
				       &pixelShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &deferredPixelShader);

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
	 dev->CreateBuffer(&matrixBufferDesc, NULL, &deferredConstantsBuffer);
}


void TearDownDeferred()
{
	deferredConstantsBuffer->Release();
	deferredVertexShader->Release();
	deferredInputLayout->Release();

	deferredPixelShader->Release();
}


void UpdateDeferred()
{
	D3DXMATRIX projectionMatrix;
	D3DXMATRIX lookAtMatrix;
	D3DXMATRIX modelViewProjectionMatrix;

	// Setup the projection matrix.
	float fieldOfView = (float)D3DX_PI / 4.0f;
	float screenAspect = (float)800 / (float)600;

	// Create the projection matrix for 3D rendering.
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, fieldOfView, screenAspect, 0.1, 1000.0);

	D3DXMatrixLookAtLH(&lookAtMatrix, &D3DXVECTOR3(0,10,-10), &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1,0));

	D3DXMatrixMultiply(&modelViewProjectionMatrix, &lookAtMatrix, &projectionMatrix);

	// setup parameters

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Lock the constant buffer so it can be written to.
	devcon->Map(deferredConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->modelViewProjection = modelViewProjectionMatrix;

	// Unlock the constant buffer.
	devcon->Unmap(deferredConstantsBuffer, 0);
}

void SetDeferredRenderer()
{
	/// setup shaders
	devcon->IASetInputLayout(deferredInputLayout);

	devcon->VSSetShader(deferredVertexShader,NULL,0);
	devcon->PSSetShader(deferredPixelShader,NULL,0);

	devcon->VSSetConstantBuffers(0, 1, &deferredConstantsBuffer);
}
