#include "pch.h"
#include "DemoApp.h"
#include "MyUtils.h"

DemoApp::DemoApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{
}


DemoApp::~DemoApp()
{
}

bool DemoApp::Init()
{
	if(!D3DApp::Init())
	{
		return false;
	}
	CreateShader();
	CreateVertexBuffer();
	CreateIndexBuffer();

	return true;
}

void DemoApp::OnResize()
{
	D3DApp::OnResize();
}

void DemoApp::UpdateScene(float dTime)
{

}

void DemoApp::DrawScene()
{
	assert(m_D3DImmediateContext);
	assert(m_SwapChain);

	float ClearColor[4] = {0.3f, 0.3f, 0.3f, 1.0f};
	m_D3DImmediateContext->ClearRenderTargetView(m_RenderTargetView, ClearColor);
	
	//Set Inoput Assembler
	m_D3DImmediateContext->IASetInputLayout(m_pVertexLayout);
	m_D3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;

	m_D3DImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_D3DImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	m_D3DImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_D3DImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);
	m_D3DImmediateContext->Draw(3, 0);
	
	HR(m_SwapChain->Present(0, 0));
}

void DemoApp::CreateShader()
{
	ID3DBlob* pErrorBlob = nullptr;
	ID3DBlob* pVSBlob = nullptr;
	HR(D3DX11CompileFromFile(
		L"MyShader.fx", 0, 0,
		"VS", "vs_5_0", 0,
		0, 0,
		&pVSBlob, &pErrorBlob, 0));

	HR(m_D3DDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(), 
		pVSBlob->GetBufferSize(), 
		0, &m_pVertexShader));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	UINT numElements = ARRAYSIZE(layout);
	HR(m_D3DDevice->CreateInputLayout(layout, numElements,
										pVSBlob->GetBufferPointer(),
										pVSBlob->GetBufferSize(),
										&m_pVertexLayout));
	pVSBlob->Release();

	ID3DBlob* pPSBlob = nullptr;
	HR(D3DX11CompileFromFile(
		L"MyShader.fx", 0, 0,
		"PS", "ps_5_0", 0,
		0, 0,
		&pPSBlob, &pErrorBlob, 0));

	HR(m_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
								   pPSBlob->GetBufferSize(),
								   0, &m_pPixelShader));

	pPSBlob->Release();
}

void DemoApp::CreateVertexBuffer()
{
	MyVertex vertices[] =
	{
		{XMFLOAT3 {-0.5f, 0.5f, 0.2f}, XMFLOAT4 {0.0f, 1.0f, 0.0f, 1.0f}},
		{XMFLOAT3 {0.5f, 0.5f, 0.2f}, XMFLOAT4 {0.0f, 1.0f, 0.0f, 1.0f}},
		{XMFLOAT3 {0.5f, -0.5f, 0.2f}, XMFLOAT4 {0.0f, 1.0f, 0.0f, 1.0f}},
		{XMFLOAT3 {-0.5f, -0.5f, 0.2f}, XMFLOAT4 {0.0f, 1.0f, 0.0f, 1.0f}},
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(vertices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(m_D3DDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer));
}

void DemoApp::CreateIndexBuffer()
{
	UINT indices[] = { 0, 1, 2, 1, 3, 2};
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	HR(m_D3DDevice->CreateBuffer(&ibd, &InitData, &m_pIndexBuffer));
}
