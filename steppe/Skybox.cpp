
#include "Globals.h"

ID3D11PixelShader * skyboxPixelShader;

ID3D11ShaderResourceView* skyboxTextureResourceView;

void SetupSkybox()
{
	ID3D10Blob* errorMessage = NULL;
	ID3D10Blob* pixelShaderBlob = NULL;


	D3DX11CompileFromFile(L"Skybox.hlsl", NULL, NULL, "SkyboxPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
		&pixelShaderBlob, &errorMessage, NULL);

	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage);
	}

	dev->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &skyboxPixelShader);

	pixelShaderBlob->Release();

	// render to texture

	D3DX11CreateShaderResourceViewFromFile(dev, L"textures/skybox.png", NULL, NULL, &skyboxTextureResourceView, NULL);
}

void TearDownSkybox()
{
	skyboxPixelShader->Release();
	skyboxTextureResourceView->Release();

}

#include "FullScreenQuad.h"

// TODO: externs are quite brittle
extern ID3D11Buffer * deferredConstantsBuffer;
extern ID3D11SamplerState * sampleStatePoint;

void RenderSkybox()
{

	devcon->PSSetShader(skyboxPixelShader,NULL,0);
	devcon->PSSetConstantBuffers(0, 1, &deferredConstantsBuffer);
	devcon->PSSetShaderResources(0, 1, &skyboxTextureResourceView);

	devcon->PSSetSamplers(0, 1, &sampleStatePoint);
	RenderFullScreenQuad();

	ID3D11ShaderResourceView * empty = NULL;

	devcon->PSSetShaderResources(0, 1, &empty);
}
