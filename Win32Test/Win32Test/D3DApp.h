#pragma once
#include "D3DUtil.h"

class GameTimer;

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE				AppInst() const;
	HWND					MainWnd() const;
	float					AspectRatio() const;

	int						Run();

	virtual bool			Init();
	virtual void			OnResize();
	virtual void			UpdateScene(float dTime) = 0;
	virtual void			DrawScene() = 0;
	virtual LRESULT			MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void			OnMouseDown(WPARAM btnState, int x, int y);
	virtual void			OnMouseUp(WPARAM btnState, int x, int y);
	virtual void			OnMouseMove(WPARAM btnState, int x, int y);

protected:
	bool					InitMainWindow();
	bool					InitDirect3D();
	void					CalculateFrameStats();

protected:
	HINSTANCE				m_AppHInstance;
	HWND					m_HWnd;
	bool					m_AppPaused;
	bool					m_Minimized;
	bool					m_Maximized;
	bool					m_Resizing;
	UINT					m_4xMsaaQuality;
	
	int						m_ClientWidth;
	int						m_ClientHeight;
	bool					m_Enable4xMsaa;

	ID3D11Device*			m_D3DDevice;
	ID3D11DeviceContext*	m_D3DImmediateContext;
	IDXGISwapChain*			m_SwapChain;
	ID3D11Texture2D*		m_DepthStencilBuffer;
	ID3D11RenderTargetView*	m_RenderTargetView;
	ID3D11DepthStencilView*	m_DepthStencilView;
	D3D11_VIEWPORT			m_ScreenViewport;
	D3D_DRIVER_TYPE			m_D3DDriverType;

	GameTimer*				m_Timer;
	std::wstring			m_MainWndCaption;
};

