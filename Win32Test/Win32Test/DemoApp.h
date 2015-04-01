#pragma once
#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"

class DemoApp : public D3DApp 
{
public:
	DemoApp(HINSTANCE hInstance);
	~DemoApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dTime);
	void DrawScene();

private:
	void CreateVertexBuffer();
	void CreateShader();
	void CreateIndexBuffer();
private:
	ID3D11VertexShader*	m_pVertexShader = nullptr;
	ID3D11PixelShader*	m_pPixelShader = nullptr;
	ID3D11InputLayout*	m_pVertexLayout = nullptr;
	ID3D11Buffer*		m_pVertexBuffer = nullptr;
	ID3D11Buffer*		m_pIndexBuffer = nullptr;
};




