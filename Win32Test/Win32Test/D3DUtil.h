#pragma once

//��ũ��
#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }
#define SafeDelete(x) { if(x)delete x; x = 0; }

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)\
{\
	HRESULT hr = (x);\
	if(FAILED(hr))\
	{\
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true);\
	}\
}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 


namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White = {1.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Black = {0.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Red = {1.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Green = {0.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Blue = {0.0f, 0.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Cyan = {0.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Magenta = {1.0f, 0.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Silver = {0.75f, 0.75f, 0.75f, 1.0f};
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};
}

class D3DUtil
{
public:
	D3DUtil();
	~D3DUtil();
};

