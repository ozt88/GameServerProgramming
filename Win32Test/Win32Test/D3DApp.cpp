#include "pch.h"
#include "D3DApp.h"
#include "GameTimer.h"

namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
	:m_AppHInstance(hInstance),
	m_MainWndCaption(L"D3D11 Application"),
	m_D3DDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_ClientWidth(800),
	m_ClientHeight(600),
	m_Enable4xMsaa(false),
	m_HWnd(0),
	m_AppPaused(false),
	m_Minimized(false),
	m_Maximized(false),
	m_Resizing(false),
	m_4xMsaaQuality(0),

	m_D3DDevice(0),
	m_D3DImmediateContext(0),
	m_SwapChain(0),
	m_DepthStencilBuffer(0),
	m_RenderTargetView(0),
	m_DepthStencilView(0)
{
	m_Timer = new GameTimer();
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
	gd3dApp = this;
}


D3DApp::~D3DApp()
{
	SafeDelete(m_Timer);
	 
	ReleaseCOM(m_RenderTargetView);
	ReleaseCOM(m_DepthStencilView);
	ReleaseCOM(m_SwapChain);
	ReleaseCOM(m_DepthStencilBuffer);

	// Restore all default settings.
	if(m_D3DImmediateContext)
		m_D3DImmediateContext->ClearState();

	ReleaseCOM(m_D3DImmediateContext);
	ReleaseCOM(m_D3DDevice);
}

HINSTANCE D3DApp::AppInst() const
{
	return m_AppHInstance; 
}

HWND D3DApp::MainWnd() const
{
	return m_HWnd;
}

float D3DApp::AspectRatio() const
{
	return static_cast<float>( m_ClientWidth ) / m_ClientHeight;
}

int D3DApp::Run()
{
	MSG msg = {0};

	m_Timer->Reset();

	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			m_Timer->Tick();

			if(!m_AppPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_Timer->DeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int) msg.wParam;
}

bool D3DApp::Init()
{
	if(!InitMainWindow())
		return false;

	if(!InitDirect3D())
		return false;

	return true;
}

void D3DApp::OnResize()
{
	assert(m_D3DImmediateContext);
	assert(m_D3DDevice);
	assert(m_SwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	ReleaseCOM(m_RenderTargetView);
	ReleaseCOM(m_DepthStencilView);
	ReleaseCOM(m_DepthStencilBuffer);


	// Resize the swap chain and recreate the render target view.

	HR(m_SwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(m_SwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &backBuffer )));
	HR(m_D3DDevice->CreateRenderTargetView(backBuffer, 0, &m_RenderTargetView));
	ReleaseCOM(backBuffer);

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if(m_Enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_D3DDevice->CreateTexture2D(&depthStencilDesc, 0, &m_DepthStencilBuffer));
	HR(m_D3DDevice->CreateDepthStencilView(m_DepthStencilBuffer, 0, &m_DepthStencilView));


	// Bind the render target view and depth/stencil view to the pipeline.

	m_D3DImmediateContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);


	// Set the viewport transform.

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>( m_ClientWidth );
	m_ScreenViewport.Height = static_cast<float>( m_ClientHeight );
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_D3DImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}

LRESULT D3DApp::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC	hdc;
	PAINTSTRUCT	ps;

	switch(message)
	{
		case WM_CREATE:
		{
			srand((unsigned) ( time(NULL) ));
			return 0;
		}

		case WM_ACTIVATE:
		{
			if(LOWORD(wParam) == WA_INACTIVE)
			{
				m_AppPaused = true;
				m_Timer->Stop();
			}
			else
			{
				m_AppPaused = false;
				m_Timer->Start();
			}
			return 0;
		}

		case WM_ENTERSIZEMOVE:
		{
			m_AppPaused = true;
			m_Resizing = true;
			m_Timer->Stop();
			return 0;
		}
		
		case WM_EXITSIZEMOVE:
		{
			m_Resizing = false;
			m_AppPaused = false;
			m_Timer->Start();
			OnResize();
			return 0;
		}

		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_COMMAND:
		{
			return 0;
		}

		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		{
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}

		case WM_MBUTTONUP:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		{
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			( (MINMAXINFO*) lParam )->ptMinTrackSize.x = 200;
			( (MINMAXINFO*) lParam )->ptMinTrackSize.y = 200;
			
			return 0;
		}
		

		case WM_MENUCHAR:
		{
			return MAKELRESULT(0, MNC_CLOSE);
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void D3DApp::OnMouseDown(WPARAM btnState, int x, int y)
{
}

void D3DApp::OnMouseUp(WPARAM btnState, int x, int y)
{
}

void D3DApp::OnMouseMove(WPARAM btnState, int x, int y)
{
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_AppHInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if(!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT rect = {0, 0, m_ClientWidth, m_ClientHeight};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_HWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
							 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_AppHInstance, 0);
	if(!m_HWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_HWnd, SW_SHOW);
	UpdateWindow(m_HWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,                 // default adapter
		m_D3DDriverType,
		0,                 // no software device
		createDeviceFlags,
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&m_D3DDevice,
		&featureLevel,
		&m_D3DImmediateContext);

	if(FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if(featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	HR(m_D3DDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));
	assert(m_4xMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_ClientWidth;
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if(m_Enable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = m_HWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	IDXGIDevice* dxgiDevice = 0;
	HR(m_D3DDevice->QueryInterface(__uuidof( IDXGIDevice ), (void**) &dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof( IDXGIAdapter ), (void**) &dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof( IDXGIFactory ), (void**) &dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(m_D3DDevice, &sd, &m_SwapChain));

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	OnResize();

	return true;
}

void D3DApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.f;

	frameCnt++;

	if(m_Timer->TotalTime() - timeElapsed >= 1.f)
	{
		float fps = (float) frameCnt;
		float mspf = 1000.f / fps;
		std::wostringstream outs;
		outs.precision(6);
		outs << m_MainWndCaption << L"   " << L"FPS: " << fps 
			<< L"   " << L"Frame Time: "<< mspf << L" (ms)";
		SetWindowText(m_HWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.f;
	}
}
