
#include "Globals.h"

#define NUM_MRTS 5

ID3D11Texture2D* renderTargetTexture[NUM_MRTS];
ID3D11RenderTargetView* renderTargetView[NUM_MRTS];
ID3D11ShaderResourceView* renderTargetResourceView[NUM_MRTS];

ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilView * depthStencilView;
ID3D11DepthStencilState * depthStencilState;

ID3D11Buffer * deferredConstantsBuffer;
ID3D11VertexShader * deferredVertexShader;
ID3D11InputLayout * deferredInputLayout;

ID3D11PixelShader * deferredPixelShader;
ID3D11RasterizerState * deferredRasterizerState;

// deferred lighting stage

ID3D11PixelShader * deferredLightingPixelShader;
ID3D11SamplerState* sampleStatePoint;


struct MatrixBufferType
{
	XMMATRIX worldViewProjectionMatrix;
//	XMMATRIX viewMatrix;
//	XMMATRIX projectionMatrix;
	XMFLOAT4 playerEyePosition;
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
		{ "TANGENTU", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, 
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, 
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, 
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

	////////////////////////////////////////
	//  deferred lighting

	D3DX11CompileFromFile(L"Deferred.hlsl", NULL, NULL, "DeferredLightingPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
		&pixelShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &deferredLightingPixelShader);

	pixelShaderBlob->Release();


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
	dev->CreateSamplerState(&samplerDesc, &sampleStatePoint);

	// render to texture

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	UINT textureWidth = SCREEN_WIDTH;
	UINT textureHeight = SCREEN_HEIGHT;

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (int i=0; i< NUM_MRTS; i++)
	{

		// Create the render target texture.
		dev->CreateTexture2D(&textureDesc, NULL, &renderTargetTexture[i]);

		// Create the render target view.
		dev->CreateRenderTargetView(renderTargetTexture[i], &renderTargetViewDesc, &renderTargetView[i]);


		// Create the shader resource view.
		dev->CreateShaderResourceView(renderTargetTexture[i], &shaderResourceViewDesc, &renderTargetResourceView[i]);
	}


	// Create depth stencil view

	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = textureWidth;
	descDepth.Height = textureHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	result=dev->CreateTexture2D( &descDepth, NULL, &depthStencilBuffer );

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	result=dev->CreateDepthStencilView(depthStencilBuffer,&descDSV,&depthStencilView);

	// Create Depth stencil state

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Create depth stencil state
	dev->CreateDepthStencilState(&dsDesc, &depthStencilState);

	D3D11_RASTERIZER_DESC rasterDesc;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	dev->CreateRasterizerState(&rasterDesc, &deferredRasterizerState);

}


void TearDownDeferred()
{
	deferredConstantsBuffer->Release();
	deferredVertexShader->Release();
	deferredInputLayout->Release();

	deferredPixelShader->Release();

	deferredRasterizerState->Release();

	// lighting stage

	sampleStatePoint->Release();
	deferredLightingPixelShader->Release();

	for (int i=0; i<NUM_MRTS; i++)
	{
		renderTargetTexture[i]->Release();
		renderTargetView[i]->Release();
		renderTargetResourceView[i]->Release();
	}

	depthStencilBuffer->Release();
	depthStencilView->Release();
	depthStencilState->Release();
}

#include "Player.h"

void UpdateDeferred()
{
	// setup parameters

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Lock the constant buffer so it can be written to.
	devcon->Map(deferredConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	//dataPtr->worldMatrix = XMMatrixTranspose(PlayerWorldMatrix());
	//dataPtr->viewMatrix = XMMatrixTranspose(PlayerViewMatrix());
	dataPtr->worldViewProjectionMatrix = XMMatrixTranspose(PlayerWorldMatrix()*PlayerViewMatrix()*PlayerProjectionMatrix());
	XMFLOAT4 playerEyePosition;
	XMStoreFloat4(&playerEyePosition,PlayerEyePosition());
	playerEyePosition.w = 0.0;
	dataPtr->playerEyePosition = playerEyePosition;

	// Unlock the constant buffer.
	devcon->Unmap(deferredConstantsBuffer, 0);
}

void SetDeferredRenderer()
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth=0.0;
	viewport.MaxDepth=1.0;

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	devcon->OMSetRenderTargets(NUM_MRTS, renderTargetView, depthStencilView);
	// Bind depth stencil state
	devcon->OMSetDepthStencilState(depthStencilState, 1);

	devcon->RSSetViewports(1, &viewport);

	for (int i=0; i<NUM_MRTS; i++)
	{
		devcon->ClearRenderTargetView(renderTargetView[i], D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	}
	devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	/// setup shaders
	devcon->IASetInputLayout(deferredInputLayout);

	devcon->VSSetShader(deferredVertexShader,NULL,0);
	devcon->GSSetShader(NULL, NULL, 0);
	devcon->PSSetShader(deferredPixelShader,NULL,0);

	devcon->VSSetConstantBuffers(0, 1, &deferredConstantsBuffer);

	devcon->RSSetState(deferredRasterizerState);
}

#include "FullScreenQuad.h"

void RenderDeferredLighting()
{

	devcon->PSSetShader(deferredLightingPixelShader,NULL,0);
	devcon->PSSetConstantBuffers(0, 1, &deferredConstantsBuffer);
	devcon->PSSetShaderResources(0, NUM_MRTS, renderTargetResourceView);

	devcon->PSSetSamplers(0, 1, &sampleStatePoint);
	RenderFullScreenQuad();

	ID3D11ShaderResourceView * empty[] = {NULL,NULL,NULL,NULL,NULL};

	devcon->PSSetShaderResources(0, NUM_MRTS, empty);
}
