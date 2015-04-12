#include "pch.h"
#include "DemoApp.h"
#include "MyUtils.h"
#define PI 3.141592f
DemoApp::DemoApp(HINSTANCE hInstance)
	:D3DApp(hInstance), m_pVertexBuffer(0), m_pIndexBuffer(0), m_FX(0), m_Tech(0), m_FxWorldViewProj(0), m_pVertexLayout(0), m_Theta(1.5f * PI), m_Phi(0.25f*PI), m_Radius(5.f)
{
	m_MainWndCaption = L"Box Demo";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_World, I);
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);
}


DemoApp::~DemoApp()
{
	ReleaseCOM(m_pVertexBuffer);
	ReleaseCOM(m_pIndexBuffer);
	ReleaseCOM(m_FX);
	ReleaseCOM(m_pVertexLayout);
}

bool DemoApp::Init()
{
	if(!D3DApp::Init())
	{
		return false;
	}
	BuildFX();
	CreateInputLayout();
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
	m_D3DImmediateContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Set Inoput Assembler
	m_D3DImmediateContext->IASetInputLayout(m_pVertexLayout);
	m_D3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;

	m_D3DImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_D3DImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&m_World);
	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX worldViewProj = world*view*proj;

	m_FxWorldViewProj->SetMatrix(reinterpret_cast<float*>( &worldViewProj ));


	D3DX11_TECHNIQUE_DESC techDesc;
	m_Tech->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_Tech->GetPassByIndex(p)->Apply(0, m_D3DImmediateContext);
		m_D3DImmediateContext->DrawIndexed(36, 0, 0);
	}

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
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*) &Colors::White},
		{XMFLOAT3(-1.0f, +1.0f, -1.0f), (const float*) &Colors::Black},
		{XMFLOAT3(+1.0f, +1.0f, -1.0f), (const float*) &Colors::Red},
		{XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*) &Colors::Green},
		{XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*) &Colors::Blue},
		{XMFLOAT3(-1.0f, +1.0f, +1.0f), (const float*) &Colors::Yellow},
		{XMFLOAT3(+1.0f, +1.0f, +1.0f), (const float*) &Colors::Cyan},
		{XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*) &Colors::Magenta}
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(m_D3DDevice->CreateBuffer(&vertexBufferDesc, &InitData, &m_pVertexBuffer));
}

void DemoApp::CreateIndexBuffer()
{
	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	HR(m_D3DDevice->CreateBuffer(&indexBufferDesc, &InitData, &m_pIndexBuffer));
}

void DemoApp::OnMouseDown(WPARAM btnState, int x, int y)
{

}

void DemoApp::OnMouseUp(WPARAM btnState, int x, int y)
{

}

void DemoApp::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void DemoApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(L"MyShader.fx", 0, 0, 0, "fx_5_0", shaderFlags,
									   0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs can store errors or warnings.
	if(compilationMsgs != 0)
	{
		MessageBoxA(0, (char*) compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	// Even if there are no compilationMsgs, check to make sure there were no other errors.
	if(FAILED(hr))
	{
		DXTrace(__FILE__, (DWORD) __LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, m_D3DDevice, &m_FX));

	// Done with compiled shader.
	ReleaseCOM(compiledShader);

	m_Tech = m_FX->GetTechniqueByName("ColorTech");
	m_FxWorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void DemoApp::CreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	m_Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_D3DDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &m_pVertexLayout));
}
