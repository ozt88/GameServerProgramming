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

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void CreateVertexBuffer();
	void CreateShader();
	void BuildFX();
	void CreateIndexBuffer();
	void CreateInputLayout();

private:
	ID3D11VertexShader*	m_pVertexShader;
	ID3D11PixelShader*	m_pPixelShader;
	ID3D11InputLayout*	m_pVertexLayout;

	ID3D11Buffer*		m_pVertexBuffer;
	ID3D11Buffer*		m_pIndexBuffer;

	ID3DX11Effect* m_FX;
	ID3DX11EffectTechnique* m_Tech;

private:
	ID3DX11EffectMatrixVariable* m_FxWorldViewProj;

	XMFLOAT4X4 m_World;
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;

	float m_Theta;
	float m_Phi;
	float m_Radius;

	POINT m_LastMousePos;
};




